#include "UpdaterState.h"
#include "common.h"
#include "utils/DownloadUtils.h"
#include "utils/UpdateUtils.h"
#include <coreinit/cache.h>

void UpdaterState::RenderError() {
    DrawUtils::setFontColor(COLOR_RED);
    DrawUtils::setFontSize(30);
    DrawUtils::print(16, 90, "An error has happened:");
    DrawUtils::setFontSize(20);
    DrawUtils::print(16, 120, ErrorMessage());
    if (this->mErrorState == ERROR_FAILED_COPY_FILES) {
        DrawUtils::print(16, 160, "Your Aroma installation might have been corrupted. Please re-download Aroma");
        DrawUtils::print(16, 180, "from " AROMA_DOWNLOAD_URL " and replace the files on the sd card.");
    } else if (this->mErrorState == ERROR_FAILED_TO_DOWNLOAD_VERSIONS || this->mErrorState == ERROR_DOWNLOAD_FAILED) {
        if (this->mResponseCode != -1) {
            DrawUtils::printf(16, 160, false, "Response code was %d", mResponseCode);
        } else {
            DrawUtils::printf(16, 160, false, "libcurl error code %d", mDownloadErrorCode);
            DrawUtils::printf(16, 180, false, "(%s)", mDownloadErrorText.c_str());
            if (mDownloadErrorCode == 60 && (mDownloadErrorText.find("BADCERT_FUTURE") != std::string::npos || mDownloadErrorText.find("BADCERT_EXPIRED") != std::string::npos)) {
                time_t now = time(nullptr);
                DrawUtils::printf(16, 220, false, "Make sure your console has the correct date");
                DrawUtils::printf(16, 240, false, "Date of the console: %s", ctime(&now));
            } else if (mDownloadErrorCode == 0x13371337) {
                DrawUtils::printf(16, 220, false, "Make sure to load this updater as .wuhb, not .rpx");
            } else if (mDownloadErrorCode == 6) {
                DrawUtils::printf(16, 220, false, "Please check the internet connection of your console.");
            }
        }
    }

    DrawUtils::setFontSize(18);
    DrawUtils::setFontColor(COLOR_WHITE);
    DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 14, "\ue000 Continue", true);
}

void UpdaterState::RenderConfirmPackages() {
    DrawUtils::setFontColor(COLOR_WHITE);
    DrawUtils::setFontSize(20);
    DrawUtils::printf(16, 80, false, "The following packages will be updated or installed:");
    uint32_t y = 120;
    for (auto &base : this->mVersionInfo) {
        for (auto &cat : base.getCategory()) {
            for (auto &package : cat.getPackages()) {
                if (package.getRequired() || package.getIsSelected()) {
                    if (package.getStatus() != VersionCheck::PackageStatus::Latest) {
                        DrawUtils::printf(16, y, false, "- %s", package.getName().c_str());
                        y += 20;
                    }
                }
            }
        }
    }
    DrawUtils::setFontSize(18);
    DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 14, "\ue000 Confirm", true);
    DrawUtils::print(16, SCREEN_HEIGHT - 14, "\ue001 Return");
}

void UpdaterState::RenderCheckIndividualFile() {
    DrawUtils::setFontColor(COLOR_WHITE);
    DrawUtils::setFontSize(20);
    if (mCurFile.has_value()) {
        DrawUtils::print(16, 80, "###### WARNING ######");
        DrawUtils::printf(16, 100, false, "File: %s", mCurFile->getPath().c_str());
        uint32_t y = 140;
        if (mCurFile->getStatus() == VersionCheck::FileStatus::Newer) {
            DrawUtils::print(16, y, "This version of this file is known, but is newer than expected.");
            y += 20;
            DrawUtils::print(16, y, "(Possibly from a nightly release?)");
            y += 20;
            DrawUtils::print(16, y, "Do you want to downgrade the file?");
            y += 40;
        } else if (mCurFile->getStatus() == VersionCheck::FileStatus::Error) {
            DrawUtils::print(16, y, "Failed to check if this version of this file is known.");
            y += 20;
            DrawUtils::print(16, y, "Do you want to overwrite the file?");
            y += 40;
        } else {
            DrawUtils::print(16, y, "This version of this file is unknown. Probably a custom build?");
            y += 20;
            DrawUtils::print(16, y, "Do you want to overwrite the file?");
            y += 40;
        }

        DrawUtils::print(16, y, "Press A to confirm");
        y += 20;
        DrawUtils::print(16, y, "Press B to skip this file");

        DrawUtils::setFontSize(18);

        DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 14, "\ue000 Confirm", true);
        DrawUtils::print(16, SCREEN_HEIGHT - 14, "\ue001 Skip");
    }
}
