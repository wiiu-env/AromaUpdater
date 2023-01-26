#include "UpdaterState.h"
#include "common.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include <sys/stat.h>

bool CheckTempFilesValid(const std::vector<VersionCheck::RepositoryFile> &filesToCheck,
                         const std::string &base_path = SD_PATH,
                         const std::string &suffix    = UPDATE_TEMP_SUFFIX);

bool CopyFilesFinal(const std::vector<VersionCheck::RepositoryFile> &filesToCheck,
                    const std::string &base_path   = SD_PATH,
                    const std::string &suffix_temp = UPDATE_TEMP_SUFFIX,
                    const std::string &suffix_old  = UPDATE_OLD_SUFFIX);

bool RenameCurrentToOld(const std::vector<VersionCheck::RepositoryFile> &filesToCheck,
                        const std::string &base_path  = SD_PATH,
                        const std::string &suffix_old = UPDATE_OLD_SUFFIX);

bool RestoreOldToCurrent(const std::vector<VersionCheck::RepositoryFile> &filesToCheck,
                         const std::string &base_path  = SD_PATH,
                         const std::string &suffix_old = UPDATE_OLD_SUFFIX);

bool RemoveTempFiles(const std::vector<VersionCheck::RepositoryFile> &filesToCheck,
                     const std::string &base_path   = SD_PATH,
                     const std::string &suffix_temp = UPDATE_TEMP_SUFFIX);

bool CheckTempFilesValid(const std::vector<VersionCheck::RepositoryFile> &filesToCheck, const std::string &base_path, const std::string &suffix) {
    bool allValid = true;
    // Make sure the downloaded files have all the correct hash.
    for (auto &file : filesToCheck) {
        auto pathOnSDCardTemp   = string_format("%s%s%s", base_path.c_str(), file.getPath().c_str(), suffix.c_str());
        auto hashOfFileOnSDCard = hashFile(pathOnSDCardTemp);
        if (hashOfFileOnSDCard.has_value()) {
            if (hashOfFileOnSDCard != file.getSha1()) {
                DEBUG_FUNCTION_LINE_WARN("File %s has a unexpected hash", pathOnSDCardTemp.c_str());
                allValid = false;
                break;
            } else {
                DEBUG_FUNCTION_LINE("File %s is valid", pathOnSDCardTemp.c_str());
            }
        } else {
            DEBUG_FUNCTION_LINE_WARN("File %s doesn't exist", pathOnSDCardTemp.c_str());
            allValid = false;
            break;
        }
    }
    return allValid;
}

bool CopyFilesFinal(const std::vector<VersionCheck::RepositoryFile> &filesToCheck, const std::string &base_path, const std::string &suffix_temp, const std::string &suffix_old) {
    for (auto &file : filesToCheck) {
        auto pathOnSDCard     = string_format("%s%s", base_path.c_str(), file.getPath().c_str());
        auto pathOnSDCardTemp = string_format("%s%s%s", base_path.c_str(), file.getPath().c_str(), suffix_temp.c_str());
        auto pathOnSDCardOld  = string_format("%s%s%s", base_path.c_str(), file.getPath().c_str(), suffix_old.c_str());

        struct stat stBuf = {};
        // Remove existing files. (Shouldn't happen)
        if (stat(pathOnSDCard.c_str(), &stBuf) >= 0 && S_ISDIR(stBuf.st_mode)) {
            DEBUG_FUNCTION_LINE("Remove %s", pathOnSDCard.c_str());
            if (remove(pathOnSDCard.c_str()) < 0) {
                DEBUG_FUNCTION_LINE_ERR("(Should not happen) Failed to remove %s", pathOnSDCard.c_str());
            }
        }
        DEBUG_FUNCTION_LINE("Rename %s to %s", pathOnSDCardTemp.c_str(), pathOnSDCard.c_str());
        if (rename(pathOnSDCardTemp.c_str(), pathOnSDCard.c_str()) < 0) {
            DEBUG_FUNCTION_LINE_ERR("Failed to rename %s to %s", pathOnSDCardTemp.c_str(), pathOnSDCard.c_str());
            return false;
        }
        if (file.getStatus() != VersionCheck::FileStatus::Missing) {
            DEBUG_FUNCTION_LINE("Remove %s", pathOnSDCardOld.c_str());
            if (remove(pathOnSDCardOld.c_str()) < 0) {
                DEBUG_FUNCTION_LINE_ERR("Failed to remove %s", pathOnSDCardOld.c_str());
            }
        }
    }
    return true;
}

bool RenameCurrentToOld(const std::vector<VersionCheck::RepositoryFile> &filesToCheck, const std::string &base_path, const std::string &suffix_old) {
    // Rename the target files to .update.old if it exists.
    bool allSuccessful = true;
    for (auto &file : filesToCheck) {
        auto pathOnSDCard    = string_format("%s%s", base_path.c_str(), file.getPath().c_str());
        auto pathOnSDCardOld = string_format("%s%s%s", base_path.c_str(), file.getPath().c_str(), suffix_old.c_str());
        // backup the files that actually exist.
        if (file.getStatus() != VersionCheck::FileStatus::Missing) {
            struct stat stBuf = {};
            // Remove any old ".temp.old" files
            if (stat(pathOnSDCardOld.c_str(), &stBuf) >= 0 && S_ISREG(stBuf.st_mode)) {
                DEBUG_FUNCTION_LINE("Remove %s", pathOnSDCardOld.c_str());
                if (remove(pathOnSDCardOld.c_str()) < 0) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to remove %s", pathOnSDCardOld.c_str());
                }
            }
            // rename file to ".temp.old"
            DEBUG_FUNCTION_LINE("Rename %s to %s", pathOnSDCard.c_str(), pathOnSDCardOld.c_str());
            if (rename(pathOnSDCard.c_str(), pathOnSDCardOld.c_str()) < 0) {
                DEBUG_FUNCTION_LINE_ERR("Failed to rename %s to %s", pathOnSDCard.c_str(), pathOnSDCardOld.c_str());
                allSuccessful = false;
                break;
            }
        }
    }
    return allSuccessful;
}

bool RestoreOldToCurrent(const std::vector<VersionCheck::RepositoryFile> &filesToCheck, const std::string &base_path, const std::string &suffix_old) {
    DEBUG_FUNCTION_LINE("Try to restore files");
    bool allSuccess = true;
    // If renaming existing files to ".temp.old" failed, try to restore
    for (auto &file : filesToCheck) {
        if (file.getStatus() != VersionCheck::FileStatus::Missing) {
            auto pathOnSDCard    = string_format("%s%s", base_path.c_str(), file.getPath().c_str());
            auto pathOnSDCardOld = string_format("%s%s%s", base_path.c_str(), file.getPath().c_str(), suffix_old.c_str());
            struct stat stBuf    = {};
            if (stat(pathOnSDCardOld.c_str(), &stBuf) >= 0 && S_ISREG(stBuf.st_mode) && stat(pathOnSDCard.c_str(), &stBuf) < 0) {
                DEBUG_FUNCTION_LINE("Try to restore %s", pathOnSDCard.c_str());
                // rename file from ".temp.old"
                DEBUG_FUNCTION_LINE("Rename %s to %s", pathOnSDCardOld.c_str(), pathOnSDCard.c_str());
                if (rename(pathOnSDCardOld.c_str(), pathOnSDCard.c_str()) < 0) {
                    allSuccess = false;
                    DEBUG_FUNCTION_LINE_ERR("Failed to rename %s to %s", pathOnSDCardOld.c_str(), pathOnSDCard.c_str());
                }
            }
        }
    }
    return allSuccess;
}

bool RemoveTempFiles(const std::vector<VersionCheck::RepositoryFile> &filesToCheck, const std::string &base_path, const std::string &suffix_temp) {
    bool allSuccess = true;
    // remove all temp files on error.
    for (auto &file : filesToCheck) {
        auto pathOnSDCardTemp = string_format("%s%s%s", base_path.c_str(), file.getPath().c_str(), suffix_temp.c_str());
        DEBUG_FUNCTION_LINE("Remove %s", pathOnSDCardTemp.c_str());
        if (remove(pathOnSDCardTemp.c_str()) < 0) {
            DEBUG_FUNCTION_LINE_WARN("Failed to remove %s", pathOnSDCardTemp.c_str());
            allSuccess = false;
        }
    }
    return allSuccess;
}

ApplicationState::eSubState UpdaterState::UpdateDownloadFilesFinished(Input *pInput) {
    if (!CheckTempFilesValid(mFilesToUpdateConfirmed)) {
        RemoveTempFiles(mFilesToUpdateConfirmed);
        setError(ERROR_DOWNLOADED_FILES_INVALID);
        return SUBSTATE_RUNNING;
    }
    if (!RenameCurrentToOld(mFilesToUpdateConfirmed)) {
        RestoreOldToCurrent(mFilesToUpdateConfirmed);
        RemoveTempFiles(mFilesToUpdateConfirmed);
        setError(ERROR_FAILED_COPY_FILES);
        return SUBSTATE_RUNNING;
    }
    if (!CopyFilesFinal(mFilesToUpdateConfirmed)) {
        RestoreOldToCurrent(mFilesToUpdateConfirmed);
        RemoveTempFiles(mFilesToUpdateConfirmed);
        setError(ERROR_FAILED_COPY_FILES);
        return SUBSTATE_RUNNING;
    }

    mState = STATE_UPDATE_SUCCESS;

    return SUBSTATE_RUNNING;
}