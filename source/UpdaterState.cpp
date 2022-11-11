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
#include "UpdaterState.h"
#include "utils/UpdateUtils.h"

UpdaterState::UpdaterState() {
    this->mState         = STATE_DOWNLOAD_VERSIONS;
    this->mErrorState    = ERROR_NONE;
    this->mLoadAnimation = {"\ue020", "\ue021", "\ue022", "\ue023", "\ue024", "\ue025", "\ue026", "\ue027"};
}

UpdaterState::~UpdaterState() = default;

void UpdaterState::setError(UpdaterState::eErrorState err) {
    this->mState      = STATE_ERROR;
    this->mErrorState = err;
}

extern "C" void OSShutdown();

ApplicationState::eSubState UpdaterState::update(Input *input) {
    switch (this->mState) {
        case STATE_ERROR: {
            if (entrySelected(input)) {
                return ApplicationState::SUBSTATE_RETURN;
            }
            return ApplicationState::SUBSTATE_RUNNING;
        }
        case STATE_DOWNLOAD_VERSIONS: {
            return UpdaterState::UpdateDownloadVersions(input);
        }
        case STATE_PARSE_VERSIONS: {
            return UpdaterState::UpdateParseVersions(input);
        }
        case STATE_CHECK_VERSIONS: {
            return UpdaterState::UpdateCheckVersions(input);
        }
        case STATE_SHOW_VERSIONS: {
            return UpdateShowVersionsMenu(input);
        }
        case STATE_SELECTED_PACKAGES_EMPTY: {
            if (buttonPressed(input, Input::BUTTON_A)) {
                mState = STATE_SHOW_VERSIONS;
            } else if (buttonPressed(input, Input::BUTTON_B)) {
                mState = STATE_SHOW_VERSIONS;
            }
            break;
        }
        case STATE_CONFIRM_PACKAGES: {

            if (buttonPressed(input, Input::BUTTON_A)) {
                mState = STATE_UPDATE_CHECK_FILES;
            } else if (buttonPressed(input, Input::BUTTON_B)) {
                mState = STATE_SHOW_VERSIONS;
            }
            break;
        }
        case STATE_UPDATE_CHECK_FILES: {
            mFilesToUpdateAll     = UpdateUtils::GetNonLatestFilesFromVersionInfo(mVersionInfo, mOnlyRequired);
            mFilesToUpdateAllSize = mFilesToUpdateAll.size();
            mState                = STATE_UPDATE_GET_INDIVIDUAL_FILE;
            break;
        }
        case STATE_UPDATE_GET_INDIVIDUAL_FILE: {
            bool fullBreak = false;
            while (!mFilesToUpdateAll.empty()) {
                mCurFile = mFilesToUpdateAll.front();
                mFilesToUpdateAll.erase(mFilesToUpdateAll.begin() + 0);
                if (mCurFile->getStatus() == VersionCheck::FileStatus::Outdated ||
                    mCurFile->getStatus() == VersionCheck::FileStatus::Missing) {
                    mFilesToUpdateConfirmed.push_back(mCurFile.value());
                    mCurFile = {};
                } else {
                    mState    = STATE_UPDATE_CHECK_INDIVIDUAL_FILE;
                    fullBreak = true;
                    break;
                }
            }

            if (fullBreak) {
                break;
            }

            mCurFile = {};
            if (!mFilesToUpdateConfirmed.empty() && (mFilesToUpdateConfirmed.size() == mFilesToUpdateAllSize)) {
                mFilesDownloadList = mFilesToUpdateConfirmed;
                mState             = STATE_UPDATE_PROCESS_DOWNLOAD_FILES;
            } else {
                mState = STATE_UPDATE_PROCESS_FILES;
            }

            break;
        }
        case STATE_UPDATE_CHECK_INDIVIDUAL_FILE: {
            if (!mCurFile.has_value()) {
                setError(ERROR_UNKNOWN);
                break;
            } else {
                if (buttonPressed(input, Input::BUTTON_A)) {
                    mFilesToUpdateConfirmed.push_back(mCurFile.value());
                    mState = STATE_UPDATE_GET_INDIVIDUAL_FILE;
                } else if (buttonPressed(input, Input::BUTTON_B)) {
                    mState = STATE_UPDATE_GET_INDIVIDUAL_FILE;
                } else {
                    break;
                }
                mCurFile = {};
            }
            break;
        }
        case STATE_UPDATE_PROCESS_FILES: {
            if (!mFilesToUpdateConfirmed.empty()) {
                if (buttonPressed(input, Input::BUTTON_A)) {
                    mFilesDownloadList = mFilesToUpdateConfirmed;
                    mState             = STATE_UPDATE_PROCESS_DOWNLOAD_FILES;
                    break;
                } else if (buttonPressed(input, Input::BUTTON_B)) {
                    return SUBSTATE_RETURN;
                }
            } else {
                mState = STATE_UPDATE_PROCESS_FILES_EMPTY;
            }
            break;
        }
        case STATE_UPDATE_PROCESS_FILES_EMPTY: {
            if (buttonPressed(input, Input::BUTTON_A)) {
                return SUBSTATE_RETURN;
            }
            break;
        }
        case STATE_UPDATE_PROCESS_DOWNLOAD_FILES: {
            return UpdaterState::UpdateProcessDownloadFiles(input);
        }
        case STATE_UPDATE_PROCESS_DOWNLOAD_FILES_FINISHED: {
            return UpdaterState::UpdateDownloadFilesFinished(input);
        }
        case STATE_UPDATE_SUCCESS: {
            if (buttonPressed(input, Input::BUTTON_A)) {
                OSShutdown();
                return SUBSTATE_SHUTDOWN;
            }
            break;
        }
    }

    return ApplicationState::SUBSTATE_RUNNING;
}


void UpdaterState::render() {
    DrawUtils::beginDraw();
    DrawUtils::clear(BACKGROUND_COLOR);
    ApplicationState::printHeader();

    switch (mState) {
        case STATE_ERROR: {
            UpdaterState::RenderError();
            break;
        }
        case STATE_DOWNLOAD_VERSIONS: {
            UpdaterState::RenderDownloadVersions();
            break;
        }
        case STATE_PARSE_VERSIONS: {
            DrawUtils::setFontColor(COLOR_WHITE);
            DrawUtils::setFontSize(20);
            DrawUtils::print(16, 80, "Parsing the latest versions...");
            break;
        }
        case STATE_CHECK_VERSIONS: {
            DrawUtils::setFontColor(COLOR_WHITE);
            DrawUtils::setFontSize(20);
            if (this->mProgress > 0.0f) {
                DrawUtils::printf(16, 80, false, "Checking the version of each file, this might take a bit... %.0f%%. %s", this->mProgress * 100.0f, this->mLoadAnimation[mAnimationTick++ % this->mLoadAnimation.size()].c_str());
            } else {
                DrawUtils::printf(16, 80, false, "Checking the version of each file, this might take a bit... %s", this->mLoadAnimation[mAnimationTick++ % this->mLoadAnimation.size()].c_str());
            }
            break;
        }
        case STATE_SHOW_VERSIONS: {
            UpdaterState::RenderShowVersions();
            break;
        }
        case STATE_SELECTED_PACKAGES_EMPTY: {
            DrawUtils::setFontColor(COLOR_WHITE);
            DrawUtils::setFontSize(20);
            DrawUtils::printf(16, 80, false, "The selected packages are already up to date");
            DrawUtils::setFontSize(18);
            DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 14, "\ue000 Return", true);
            break;
        }
        case STATE_CONFIRM_PACKAGES: {
            UpdaterState::RenderConfirmPackages();
            break;
        }
        case STATE_UPDATE_CHECK_FILES: {
            DrawUtils::setFontColor(COLOR_WHITE);
            DrawUtils::setFontSize(20);
            DrawUtils::print(16, 80, "Checking files..");
            break;
        }
        case STATE_UPDATE_GET_INDIVIDUAL_FILE: {
            DrawUtils::setFontColor(COLOR_WHITE);
            DrawUtils::setFontSize(20);
            DrawUtils::print(16, 80, "Checking files...");
            break;
        }
        case STATE_UPDATE_CHECK_INDIVIDUAL_FILE: {
            UpdaterState::RenderCheckIndividualFile();
            break;
        }
        case STATE_UPDATE_PROCESS_FILES: {
            DrawUtils::setFontColor(COLOR_WHITE);
            DrawUtils::setFontSize(20);
            DrawUtils::printf(16, 80, false, "You have skipped %d files.", mFilesToUpdateAllSize - mFilesToUpdateConfirmed.size());
            DrawUtils::printf(16, 100, false, "Do you want to continue?");
            DrawUtils::setFontSize(18);
            DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 14, "\ue000 Confirm", true);
            DrawUtils::print(16, SCREEN_HEIGHT - 14, "\ue001 Abort");
            break;
        }
        case STATE_UPDATE_PROCESS_FILES_EMPTY:
            DrawUtils::setFontColor(COLOR_WHITE);
            DrawUtils::setFontSize(20);
            DrawUtils::print(16, 80, "All files that could be updated have been skipped.");
            DrawUtils::setFontSize(18);
            DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 14, "\ue000 Return", true);
            break;
        case STATE_UPDATE_PROCESS_DOWNLOAD_FILES: {
            UpdaterState::RenderProcessDownloadFiles();
            break;
        }
        case STATE_UPDATE_PROCESS_DOWNLOAD_FILES_FINISHED: {
            DrawUtils::setFontColor(COLOR_WHITE);
            DrawUtils::setFontSize(20);
            DrawUtils::print(16, 80, "Copy files...");
            break;
        }
        case STATE_UPDATE_SUCCESS: {
            DrawUtils::setFontSize(20);
            DrawUtils::print(16, 80, "All files have been updated successfully.");
            DrawUtils::print(16, 100, "The console will now shutdown.");
            DrawUtils::setFontSize(18);
            DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 14, "\ue000 Shutdown", true);
            break;
        }
    }

    ApplicationState::printFooter();
    DrawUtils::endDraw();
}

const char *UpdaterState::ErrorMessage() const {
    switch (this->mErrorState) {
        case ERROR_NONE:
            return "ERROR_NONE";
        case ERROR_FAILED_TO_DOWNLOAD_VERSIONS:
            return "Failed to download the latest version information.";
        case ERROR_FAILED_TO_PARSE_VERSIONS:
            return "Failed to parse the latest version information.";
        case ERROR_FAILED_TO_SAVE_FILE:
            return "Failed to write a file to the sd card.";
        case ERROR_FAILED_TO_FIND_FILE_IN_ZIP:
            return "Failed to find the file in the zip.";
        case ERROR_UNKNOWN:
            return "Something unexpected has happened.";
        case ERROR_DOWNLOAD_FAILED:
            return "The download has failed";
        case ERROR_EXTRACT_FAILED:
            return "Extracting a file to the sd card has failed.";
        case ERROR_FAILED_TO_CREATE_THREAD:
            return "Creating a thread has failed.";
        case ERROR_DOWNLOADED_FILES_INVALID:
            return "The downloaded files were corrupt.";
        case ERROR_FAILED_COPY_FILES:
            return "Failed to finalize the update.";
        case ERROR_FAILED_TO_CREATE_DIR:
            return "Failed to create a directory.";
    }
    return "Something unexpected has happened.";
}

const char *UpdaterState::ErrorDescription() const {
    switch (this->mErrorState) {
        case ERROR_NONE:
            return "ERROR_NONE";
        case ERROR_FAILED_TO_DOWNLOAD_VERSIONS:
            return "ERROR_FAILED_TO_DOWNLOAD_VERSIONS";
        case ERROR_FAILED_TO_PARSE_VERSIONS:
            return "ERROR_FAILED_TO_PARSE_VERSIONS";
        case ERROR_FAILED_TO_SAVE_FILE:
            return "ERROR_FAILED_TO_SAVE_FILE";
        case ERROR_FAILED_TO_FIND_FILE_IN_ZIP:
            return "ERROR_FAILED_TO_FIND_FILE_IN_ZIP";
        case ERROR_UNKNOWN:
            return "ERROR_UNKNOWN";
        case ERROR_DOWNLOAD_FAILED:
            return "ERROR_DOWNLOAD_FAILED";
        case ERROR_EXTRACT_FAILED:
            return "ERROR_EXTRACT_FAILED";
        case ERROR_FAILED_TO_CREATE_THREAD:
            return "ERROR_FAILED_TO_CREATE_THREAD";
        case ERROR_DOWNLOADED_FILES_INVALID:
            return "ERROR_DOWNLOADED_FILES_INVALID";
        case ERROR_FAILED_COPY_FILES:
            return "ERROR_FAILED_COPY_FILES";
        case ERROR_FAILED_TO_CREATE_DIR:
            return "ERROR_FAILED_TO_CREATE_DIR";
    }
    return "UNKNOWN_ERROR";
}
