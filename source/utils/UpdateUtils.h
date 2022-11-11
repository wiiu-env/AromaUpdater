#pragma once

#include "../common.h"
#include "VersionCheck.h"
#include <string>
#include <vector>

class UpdateUtils {
public:
    static void CheckFilesOfVersionInfo(VersionCheck::VersionInfo &versionInfo,
                                        bool onlyRequired,
                                        float *progress,
                                        const std::string &sdBase    = SD_PATH,
                                        const std::string &serverURL = UPDATE_SERVER_URL);

    static std::vector<VersionCheck::RepositoryFile> GetNonLatestFilesFromVersionInfo(const VersionCheck::VersionInfo &versionInfo, bool onlyRequired);

    static const char *StatusToString(VersionCheck::RepoStatus e) {
        switch (e) {
            case VersionCheck::RepoStatus::Latest:
                return "Latest";
            case VersionCheck::RepoStatus::Dirty:
                return "Dirty";
            case VersionCheck::RepoStatus::Missing:
                return "Missing";
        }
        return "Out of Range";
    }

    static const char *StatusToString(VersionCheck::FileStatus e) {
        switch (e) {
            case VersionCheck::FileStatus::Latest:
                return "Latest";
            case VersionCheck::FileStatus::Missing:
                return "Missing";
            case VersionCheck::FileStatus::Outdated:
                return "Outdated";
            case VersionCheck::FileStatus::UnknownFile:
                return "UnknownFile";
            case VersionCheck::FileStatus::Error:
                return "Error";
            case VersionCheck::FileStatus::Newer:
                return "Newer";
        }
        return "Out of Range";
    }

    static const char *StatusToString(VersionCheck::PackageStatus e) {
        switch (e) {
            case VersionCheck::PackageStatus::Latest:
                return "Up to date";
            case VersionCheck::PackageStatus::Dirty:
                return "Has update";
            case VersionCheck::PackageStatus::Missing:
                return "Not installed";
        }
        return "Out of Range";
    }
};
