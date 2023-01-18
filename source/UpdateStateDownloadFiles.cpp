#include "UpdaterState.h"
#include "utils/DownloadUtils.h"
#include "utils/FSUtils.h"
#include "utils/logger.h"
#include "utils/zip_file.hpp"
#include <coreinit/cache.h>
#include <coreinit/thread.h>
#include <mutex>

int DownloadFilesThreadEntry(UpdaterState *updater) {
    std::lock_guard<std::mutex> lock(updater->mFilesDownloadListLock);
    {
        std::lock_guard<std::mutex> lockInfo(updater->mDownloadInfosLock);
        updater->mDownloadInfos = DownloadInfos(updater->mFilesDownloadList.size());
    }
    for (auto &curFile : updater->mFilesDownloadList) {
        DEBUG_FUNCTION_LINE("Download and extract %s", curFile.getPath().c_str());
        {
            std::lock_guard<std::mutex> lockInfo(updater->mDownloadInfosLock);
            updater->mDownloadInfos->curFile     = curFile;
            updater->mDownloadInfos->processStep = DownloadInfos::STEP_DOWNLOAD;
        }
        OSMemoryBarrier();
        auto &curURL = curFile.getAsset().getBrowserDownloadUrl();
        DEBUG_FUNCTION_LINE("Check if %s is in cache", curURL.c_str());
        if (!updater->mZipDownloadCache.contains(curURL)) {
            std::string downloadedZIP;
            int responseCode;
            updater->mProgress = 0.0f;
            DEBUG_FUNCTION_LINE("Download %s", curURL.c_str());
            int errorCode;
            std::string errorText;
            if (DownloadUtils::DownloadFileToBuffer(curURL, downloadedZIP, responseCode, errorCode, errorText, &updater->mProgress) < 0 || responseCode != 200) {
                DEBUG_FUNCTION_LINE_ERR("Download failed");
                {
                    std::lock_guard<std::mutex> lockInfo(updater->mDownloadInfosLock);
                    updater->mDownloadInfos->state        = DownloadInfos::DOWNLOAD_FAILED;
                    updater->mDownloadInfos->errorCode    = errorCode;
                    updater->mDownloadInfos->errorText    = errorText;
                    updater->mDownloadInfos->responseCode = responseCode;
                    OSMemoryBarrier();
                    return 0;
                }
            }
            updater->mZipDownloadCache[curURL] = downloadedZIP;
        } else {
            DEBUG_FUNCTION_LINE("Use cached version.");
        }
        {
            std::lock_guard<std::mutex> lockInfo(updater->mDownloadInfosLock);
            updater->mDownloadInfos->processStep = DownloadInfos::STEP_EXTRACT;
        }
        auto &curBuffer = updater->mZipDownloadCache[curURL];
        std::istringstream iss(curBuffer);
        miniz_cpp::zip_file zip(iss);

        bool found = false;
        // find asset
        for (auto &assetFile : curFile.getAsset().getFiles()) {
            if (assetFile.getSha1() == curFile.getSha1()) {
                for (auto &member : zip.infolist()) {
                    if (member.filename == assetFile.getPath()) {
                        found                      = true;
                        auto fullPath              = SD_PATH + curFile.getPath() + UPDATE_TEMP_SUFFIX;
                        std::filesystem::path path = fullPath;

                        if (CreateSubfolder(path.remove_filename().c_str()) == 0) {
                            std::lock_guard<std::mutex> lockInfo(updater->mDownloadInfosLock);
                            updater->mDownloadInfos->state = DownloadInfos::DOWNLOAD_CREATE_DIR_FAILED;
                            OSMemoryBarrier();
                            return 0;
                        }
                        DEBUG_FUNCTION_LINE("Extract %s to %s", member.filename.c_str(), fullPath.c_str());
                        std::string ss = zip.read(member);
                        if (saveBufferToFile(fullPath.c_str(), (void *) ss.c_str(), ss.length()) < 0) {
                            {
                                std::lock_guard<std::mutex> lockInfo(updater->mDownloadInfosLock);
                                updater->mDownloadInfos->state = DownloadInfos::DOWNLOAD_EXTRACT_FAILED;
                                OSMemoryBarrier();
                                return 0;
                            }
                        }
                        DEBUG_FUNCTION_LINE("Extract done");
                        break;
                    }
                }
                break;
            }
        }

        if (!found) {
            DEBUG_FUNCTION_LINE_ERR("Failed to find file in zip");
            {
                std::lock_guard<std::mutex> lockInfo(updater->mDownloadInfosLock);
                updater->mDownloadInfos->state = DownloadInfos::DOWNLOAD_NOT_FOUND_IN_ZIP;
                OSMemoryBarrier();
            }
            return 0;
        }
        {
            std::lock_guard<std::mutex> lockInfo(updater->mDownloadInfosLock);
            updater->mDownloadInfos->filesDone++;
            OSMemoryBarrier();
        }
    }
    {
        std::lock_guard<std::mutex> lockInfo(updater->mDownloadInfosLock);
        updater->mDownloadInfos->state = DownloadInfos::DOWNLOAD_SUCCESS;
        OSMemoryBarrier();
    }

    return 0;
}

ApplicationState::eSubState UpdaterState::UpdateProcessDownloadFiles(Input *input) {
    if (mDownloadFilesThread == nullptr) {
        OSMemoryBarrier();
        mDownloadFilesThread = new (std::nothrow) std::thread(DownloadFilesThreadEntry, this);
        if (mDownloadFilesThread == nullptr) {
            setError(ERROR_FAILED_TO_CREATE_THREAD);
            return ApplicationState::SUBSTATE_RUNNING;
        }
        auto nativeHandle = (OSThread *) mDownloadFilesThread->native_handle();
        OSSetThreadName(nativeHandle, "DownloadFilesThread");
        return ApplicationState::SUBSTATE_RUNNING;
    }
    {
        std::lock_guard<std::mutex> lockInfo(this->mDownloadInfosLock);
        if (!this->mDownloadInfos.has_value() || mDownloadInfos->state == DownloadInfos::DOWNLOAD_RUNNING) {
            return ApplicationState::SUBSTATE_RUNNING;
        }
    }
    mDownloadFilesThread->join();
    delete mDownloadFilesThread;
    mDownloadFilesThread = nullptr;
    OSMemoryBarrier();

    switch (mDownloadInfos->state) {
        case DownloadInfos::DOWNLOAD_RUNNING:
            setError(ERROR_UNKNOWN);
            break;
        case DownloadInfos::DOWNLOAD_SUCCESS:
            this->mState = STATE_UPDATE_PROCESS_DOWNLOAD_FILES_FINISHED;
            break;
        case DownloadInfos::DOWNLOAD_FAILED:
            if (this->mDownloadInfos) {
                this->mResponseCode      = this->mDownloadInfos->responseCode;
                this->mDownloadErrorCode = this->mDownloadInfos->errorCode;
                this->mDownloadErrorText = this->mDownloadInfos->errorText;
            }
            setError(ERROR_DOWNLOAD_FAILED);
            break;
        case DownloadInfos::DOWNLOAD_EXTRACT_FAILED:
            setError(ERROR_EXTRACT_FAILED);
            break;
        case DownloadInfos::DOWNLOAD_NOT_FOUND_IN_ZIP:
            setError(ERROR_FAILED_TO_FIND_FILE_IN_ZIP);
            break;
        case DownloadInfos::DOWNLOAD_CREATE_DIR_FAILED:
            setError(ERROR_FAILED_TO_CREATE_DIR);
            break;
    }
    return ApplicationState::SUBSTATE_RUNNING;
}

void UpdaterState::RenderProcessDownloadFiles() {
    DrawUtils::setFontColor(COLOR_WHITE);
    DrawUtils::setFontSize(20);
    {
        std::lock_guard<std::mutex> lock(mDownloadInfosLock);
        if (mDownloadInfos.has_value()) {
            DrawUtils::printf(16, 80, false, "Downloading and extracting files... %s", this->mLoadAnimation[mAnimationTick++ % this->mLoadAnimation.size()].c_str());
            if (mDownloadInfos->curFile.has_value()) {
                DrawUtils::printf(16, 100, false, "File: %s", mDownloadInfos->curFile->getPath().c_str());
                if (mDownloadInfos->processStep == DownloadInfos::STEP_DOWNLOAD) {
                    if (this->mProgress > 0.0f) {
                        DrawUtils::printf(16, 120, false, "Downloading... %.0f%%", this->mProgress * 100.0f);
                    } else {
                        DrawUtils::printf(16, 120, false, "Downloading...");
                    }
                } else {
                    DrawUtils::printf(16, 120, false, "Extracting...", mDownloadInfos->curFile->getPath().c_str());
                }
            }
            DrawUtils::printf(16, 160, false, "%d of %d files done.", mDownloadInfos->filesDone, mDownloadInfos->totalFiles);
        } else {
            DrawUtils::printf(16, 80, false, "Preparing download of files...");
        }
    }
}
