#include "UpdaterState.h"
#include "common.h"
#include "utils/DownloadUtils.h"
#include "utils/UpdateUtils.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include <coreinit/cache.h>
#include <coreinit/thread.h>

int DownloadVersionInfoThreadEntry(UpdaterState *updater) {
    std::lock_guard<std::mutex> lock(updater->mVersionBufferLock);
    updater->mProgress = 0.0f;
    std::string url    = UPDATE_SERVER_URL "/api/check_versions";
    if (DownloadUtils::DownloadFileToBuffer(url,
                                            updater->mVersionBuffer,
                                            updater->mResponseCode,
                                            updater->mDownloadErrorCode,
                                            updater->mDownloadErrorText,
                                            &updater->mProgress) < 0 ||
        updater->mResponseCode != 200) {
        DEBUG_FUNCTION_LINE_ERR("Error while downloading \"%s\".", url.c_str());
        DEBUG_FUNCTION_LINE_ERR("curl error code: %d, curl error text %s, response code: %d",
                                updater->mDownloadErrorCode,
                                updater->mDownloadErrorText.c_str(),
                                updater->mResponseCode);
        updater->mDownloadInfoResult = UpdaterState::DOWNLOAD_FAILED;
    } else {
        updater->mDownloadInfoResult = UpdaterState::DOWNLOAD_SUCCESS;
    }

    OSMemoryBarrier();

    return 0;
}

ApplicationState::eSubState UpdaterState::UpdateDownloadVersions(Input *input) {
    if (mDownloadInfoThread == nullptr) {
        mDownloadInfoResult = DOWNLOAD_RUNNING;
        mDownloadInfoThread = new (std::nothrow) std::thread(DownloadVersionInfoThreadEntry, this);
        if (mDownloadInfoThread == nullptr) {
            mDownloadInfoResult = DOWNLOAD_FAILED;
            setError(ERROR_FAILED_TO_CREATE_THREAD);
            return ApplicationState::SUBSTATE_RUNNING;
        }
        auto nativeHandle = (OSThread *) mDownloadInfoThread->native_handle();
        OSSetThreadName(nativeHandle, "DownloadVersionInfoThread");
        OSMemoryBarrier();

        return ApplicationState::SUBSTATE_RUNNING;
    }
    if (mDownloadInfoResult == DOWNLOAD_RUNNING) {
        return ApplicationState::SUBSTATE_RUNNING;
    }

    mDownloadInfoThread->join();
    delete mDownloadInfoThread;
    mDownloadInfoThread = nullptr;
    OSMemoryBarrier();

    if (mDownloadInfoResult == DOWNLOAD_SUCCESS) {
        this->mState = STATE_PARSE_VERSIONS;
    } else {
        setError(ERROR_FAILED_TO_DOWNLOAD_VERSIONS);
    }

    return ApplicationState::SUBSTATE_RUNNING;
}


ApplicationState::eSubState UpdaterState::UpdateParseVersions(Input *input) {
    try {
        std::lock_guard<std::mutex> lock(mVersionInfoLock);
        std::lock_guard<std::mutex> lock1(mVersionBufferLock);
        mVersionInfo = nlohmann::json::parse(mVersionBuffer.c_str());
        mVersionBuffer.clear();
    } catch (std::exception &e) {
        this->setError(ERROR_FAILED_TO_PARSE_VERSIONS);
        return SUBSTATE_RUNNING;
    }
    this->mState = STATE_CHECK_VERSIONS;
    return SUBSTATE_RUNNING;
}

int CheckIndividualFileVersions(UpdaterState *updater) {
    std::lock_guard<std::mutex> lock(updater->mVersionInfoLock);
    UpdateUtils::CheckFilesOfVersionInfo(updater->mVersionInfo, updater->mOnlyRequired, &updater->mProgress);

    updater->mCheckFilesDone = true;
    OSMemoryBarrier();

    return 0;
}

ApplicationState::eSubState UpdaterState::UpdateCheckVersions(Input *input) {
    if (mCheckFilesThread == nullptr) {
        OSMemoryBarrier();

        mCheckFilesThread = new (std::nothrow) std::thread(CheckIndividualFileVersions, this);
        if (mCheckFilesThread == nullptr) {
            setError(ERROR_FAILED_TO_CREATE_THREAD);
            return SUBSTATE_RUNNING;
        }
        auto nativeHandle = (OSThread *) mCheckFilesThread->native_handle();
        OSSetThreadName(nativeHandle, "CheckFilesOfVersionInfoThread");
        return SUBSTATE_RUNNING;
    }

    if (!mCheckFilesDone) {
        return SUBSTATE_RUNNING;
    }

    mCheckFilesThread->join();
    delete mCheckFilesThread;
    mCheckFilesThread = nullptr;
    OSMemoryBarrier();

    std::lock_guard<std::mutex> lock(mVersionInfoLock);
    int32_t packageCount = 0;
    for (auto &base : mVersionInfo) {
        for (auto &cat : base.getCategory()) {
            packageCount += cat.getPackagesCount(mOnlyRequired);
        }
    }
    mTotalPackageCount = packageCount;


    // Use a one Base Category per page.
    std::vector<VersionCheck::BaseCategory> mPages;
    constexpr float maxHeightForPage = 15.0f;
    VersionCheck::BaseCategory curPage;
    auto curPageName = string_format("Page %d", 1);
    curPage.setName(curPageName);
    float curHeight = 0.0f;
    bool isFirstCat = true;

    for (auto &baseCategory : mVersionInfo) {
        for (auto &category : baseCategory.getCategory()) {
            if (category.getPackagesCount(mOnlyRequired) == 0) {
                continue;
            }

            if (!isFirstCat) {
                curHeight += 1.5f;
            }
            curHeight++;

            auto curPackageCount = (float) category.getPackagesCount(mOnlyRequired);

            if (curHeight + curPackageCount <= maxHeightForPage) {
                curPage.getMutableCategory().push_back(category);
                curHeight += curPackageCount;
                isFirstCat = false;
            } else {
                mPages.push_back(curPage);
                curPage            = {};
                auto curPageNameIn = string_format("Page %d", mPages.size() + 1);
                curPage.setName(curPageNameIn);
                curPage.getMutableCategory().push_back(category);
                curHeight  = 1.0f + curPackageCount;
                isFirstCat = false;
            }
        }
    }
    mPages.push_back(curPage);
    mVersionInfo = mPages;

    mState = STATE_SHOW_VERSIONS;

    return SUBSTATE_RUNNING;
}

void UpdaterState::RenderDownloadVersions() {
    DrawUtils::setFontColor(COLOR_WHITE);
    DrawUtils::setFontSize(20);
    if (mDownloadInfoThread == nullptr || this->mProgress == 0.0f) {
        DrawUtils::printf(16, 80, false, "Getting the latest versions... %s", this->mLoadAnimation[mAnimationTick++ % this->mLoadAnimation.size()].c_str());
    } else {
        DrawUtils::printf(16, 80, false, "Getting the latest versions... %.0f%% %s", this->mProgress * 100.0f, this->mLoadAnimation[mAnimationTick++ % this->mLoadAnimation.size()].c_str());
    }
}
