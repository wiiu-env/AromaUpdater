/****************************************************************************
 * Copyright (C) 2021 Maschell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#pragma once

#include "ApplicationState.h"
#include "input/Input.h"
#include "utils/VersionCheck.h"
#include <array>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <thread>

class DownloadInfos {
public:
    explicit DownloadInfos(uint32_t totalFiles) : totalFiles(totalFiles) {
        this->state = DOWNLOAD_RUNNING;
    }

    enum eDownloadFileState {
        DOWNLOAD_RUNNING,
        DOWNLOAD_SUCCESS,
        DOWNLOAD_FAILED,
        DOWNLOAD_EXTRACT_FAILED,
        DOWNLOAD_CREATE_DIR_FAILED,
        DOWNLOAD_NOT_FOUND_IN_ZIP,
    };
    enum eProcessStep {
        STEP_DOWNLOAD,
        STEP_EXTRACT,
    };

    uint32_t filesDone  = 0;
    uint32_t totalFiles = 0;
    std::optional<VersionCheck::RepositoryFile> curFile;
    eProcessStep processStep = STEP_DOWNLOAD;
    eDownloadFileState state = DOWNLOAD_RUNNING;
    int errorCode            = {};
    int responseCode         = {};
    std::string errorText    = {};
};

class UpdaterState : public ApplicationState {
public:
    enum eDumpState {
        STATE_ERROR,
        STATE_DOWNLOAD_VERSIONS,
        STATE_PARSE_VERSIONS,
        STATE_CHECK_VERSIONS,
        STATE_SHOW_VERSIONS,
        STATE_SELECTED_PACKAGES_EMPTY,
        STATE_CONFIRM_PACKAGES,
        STATE_UPDATE_CHECK_FILES,
        STATE_UPDATE_GET_INDIVIDUAL_FILE,
        STATE_UPDATE_CHECK_INDIVIDUAL_FILE,
        STATE_UPDATE_PROCESS_FILES,
        STATE_UPDATE_PROCESS_FILES_EMPTY,
        STATE_UPDATE_PROCESS_DOWNLOAD_FILES,
        STATE_UPDATE_PROCESS_DOWNLOAD_FILES_FINISHED,
        STATE_UPDATE_SUCCESS
    };

    enum eErrorState {
        ERROR_NONE,
        ERROR_DOWNLOAD_FAILED,
        ERROR_EXTRACT_FAILED,
        ERROR_FAILED_TO_DOWNLOAD_VERSIONS,
        ERROR_FAILED_TO_PARSE_VERSIONS,
        ERROR_FAILED_TO_SAVE_FILE,
        ERROR_FAILED_TO_FIND_FILE_IN_ZIP,
        ERROR_FAILED_TO_CREATE_THREAD,
        ERROR_DOWNLOADED_FILES_INVALID,
        ERROR_FAILED_COPY_FILES,
        ERROR_FAILED_TO_CREATE_DIR,
        ERROR_UNKNOWN
    };

    enum eDownloadState {
        DOWNLOAD_RUNNING,
        DOWNLOAD_SUCCESS,
        DOWNLOAD_FAILED,
    };

    explicit UpdaterState();

    ~UpdaterState() override;

    void setError(eErrorState err);

    void render() override;

    void RenderError();
    void RenderDownloadVersions();
    void RenderShowVersions();
    void RenderConfirmPackages();
    void RenderCheckIndividualFile();
    void RenderProcessDownloadFiles();

    ApplicationState::eSubState UpdateProcessDownloadFiles(Input *input);
    ApplicationState::eSubState UpdateDownloadVersions(Input *input);
    ApplicationState::eSubState UpdateParseVersions(Input *input);
    ApplicationState::eSubState UpdateCheckVersions(Input *input);
    ApplicationState::eSubState UpdateShowVersionsMenu(Input *input);

    eSubState update(Input *input) override;

    [[nodiscard]] const char *ErrorMessage() const;

    [[nodiscard]] const char *ErrorDescription() const;

    eDumpState mState;

    std::map<std::string, std::string> mZipDownloadCache;

    std::mutex mVersionBufferLock;
    std::string mVersionBuffer;
    int mResponseCode;
    eErrorState mErrorState;

    VersionCheck::VersionInfo mVersionInfo;

    int mDownloadErrorCode;
    std::string mDownloadErrorText;

    bool mOnlyRequired         = false;
    int32_t mTotalPackageCount = 0;

    std::vector<VersionCheck::RepositoryFile> mFilesToUpdateAll;
    uint32_t mFilesToUpdateAllSize = 0;
    std::vector<VersionCheck::RepositoryFile> mFilesToUpdateConfirmed;
    std::mutex mFilesDownloadListLock;
    std::vector<VersionCheck::RepositoryFile> mFilesDownloadList;
    std::mutex mDownloadInfosLock;
    std::optional<DownloadInfos> mDownloadInfos;

    std::optional<VersionCheck::RepositoryFile> mCurFile = {};

    std::mutex mVersionInfoLock;
    std::thread *mCheckFilesThread    = nullptr;
    std::thread *mDownloadInfoThread  = nullptr;
    std::thread *mDownloadFilesThread = nullptr;
    bool mCheckFilesDone              = false;
    uint8_t mAnimationTick            = 0;

    eDownloadState mDownloadInfoResult = DOWNLOAD_RUNNING;

    std::vector<std::string> mLoadAnimation;
    float mProgress{};
    ApplicationState::eSubState UpdateDownloadFilesFinished(Input *pInput);
};