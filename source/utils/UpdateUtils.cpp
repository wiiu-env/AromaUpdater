#include "UpdateUtils.h"
#include "DownloadUtils.h"
#include "VersionByHash.h"
#include "logger.h"
#include "utils.h"
#include <coreinit/cache.h>

void UpdateUtils::CheckFilesOfVersionInfo(VersionCheck::VersionInfo &versionInfo, bool onlyRequired, float *progress, const std::string &sdBase, const std::string &serverURL) {
    if (progress) {
        *progress = 0.0f;
    }

    uint32_t totalFiles       = 0;
    uint32_t curFileProcessed = 0;
    for (auto &baseCategory : versionInfo) {
        totalFiles += baseCategory.getFilesCount(onlyRequired);
    }

    for (auto &baseCategory : versionInfo) {
        for (auto &category : baseCategory.getMutableCategory()) {
            for (auto &package : category.getMutablePackages()) {
                if (!package.getRequired() && onlyRequired) {
                    continue;
                }
                std::string packageDate = "2000-01-01T01:01:01Z";
                for (auto &repo : package.getMutableRepositories()) {
                    std::string repoDate = "2000-01-01T01:01:01Z";
                    for (auto &file : repo.getMutableFiles()) {
                        if (progress) {
                            *progress = (float) curFileProcessed / (float) totalFiles;
                            OSMemoryBarrier();
                        }

                        if (file.getRelease().getPublishedAt() > repoDate) {
                            repoDate = file.getRelease().getPublishedAt();
                        }

                        auto pathOnSDCard       = string_format("%s%s", sdBase.c_str(), file.getPath().c_str());
                        auto hashOfFileOnSDCard = hashFile(pathOnSDCard);
                        if (hashOfFileOnSDCard) {
                            if (hashOfFileOnSDCard == file.getSha1()) {
                                file.setStatus(VersionCheck::FileStatus::Latest);
                            } else {
                                std::string outBuffer2;
                                int responseCode;

                                std::string url = string_format("%s/api/check_versions?hash=%s", serverURL.c_str(), hashOfFileOnSDCard->c_str());
                                DEBUG_FUNCTION_LINE("Check version of file with hash %s", hashOfFileOnSDCard->c_str());
                                int errorOut;
                                std::string errorTextOut;
                                if (DownloadUtils::DownloadFileToBuffer(url.c_str(), outBuffer2, responseCode, errorOut, errorTextOut, nullptr) < 0 &&
                                    responseCode == 200) {
                                    DEBUG_FUNCTION_LINE("Failed to get information for the file");
                                    file.setStatus(VersionCheck::FileStatus::Error);
                                    curFileProcessed++;
                                    continue;
                                }
                                if (outBuffer2 == "not found") {
                                    DEBUG_FUNCTION_LINE("File with this hash not found");
                                    file.setStatus(VersionCheck::FileStatus::UnknownFile);
                                    curFileProcessed++;
                                    continue;
                                }
                                try {
                                    VersionByHash::VersionInfo versionInfoSingle = nlohmann::json::parse(outBuffer2.c_str());
                                    DEBUG_FUNCTION_LINE("File with this hash found!");
                                    if (versionInfoSingle.getRelease().getPublishedAt() < file.getRelease().getPublishedAt()) {
                                        DEBUG_FUNCTION_LINE("And it's older than expected");
                                        file.setStatus(VersionCheck::FileStatus::Outdated);
                                    } else {
                                        DEBUG_FUNCTION_LINE("But it's newer than expected");
                                        file.setStatus(VersionCheck::FileStatus::Newer);
                                    }
                                } catch (std::exception &e) {
                                    DEBUG_FUNCTION_LINE_WARN("Failed to parse VersionByHash::VersionInfo");
                                    file.setStatus(VersionCheck::FileStatus::Error);
                                }
                            }
                        } else {
                            DEBUG_FUNCTION_LINE("File %s was not found on the sd card", file.getPath().c_str());
                            file.setStatus(VersionCheck::FileStatus::Missing);
                        }
                        curFileProcessed++;
                    }
                    if (progress) {
                        *progress = (float) curFileProcessed / (float) totalFiles;
                        OSMemoryBarrier();
                    }
                    bool allMissing = true;
                    for (auto &file : repo.getFiles()) {
                        if (file.getStatus() != VersionCheck::FileStatus::Missing) {
                            allMissing = false;
                        }
                        if (file.getStatus() != VersionCheck::FileStatus::Latest) {
                            repo.setStatus(VersionCheck::RepoStatus::Dirty);
                        }
                    }
                    if (allMissing) {
                        repo.setStatus(VersionCheck::RepoStatus::Missing);
                    }
                    if (repoDate > packageDate) {
                        packageDate = repoDate;
                    }
                }

                bool allMissing = true;
                for (auto &repo : package.getRepositories()) {
                    if (repo.getStatus() != VersionCheck::RepoStatus::Missing) {
                        allMissing = false;
                    }
                    if (repo.getStatus() != VersionCheck::RepoStatus::Latest) {
                        package.setStatus(VersionCheck::PackageStatus::Dirty);
                        package.setSelected(true);
                    }
                }
                if (allMissing) {
                    package.setStatus(VersionCheck::PackageStatus::Missing);
                    package.setSelected(false);
                }
                package.setLastUpdateDate(packageDate);
            }
        }
    }
}

std::vector<VersionCheck::RepositoryFile> UpdateUtils::GetNonLatestFilesFromVersionInfo(const VersionCheck::VersionInfo &versionInfo, bool onlyRequired) {
    std::vector<VersionCheck::RepositoryFile> result;
    for (auto &baseCategory : versionInfo) {
        for (auto &category : baseCategory.getCategory()) {
            for (auto &package : category.getPackages()) {
                if (!package.getIsSelected() && !package.getRequired()) {
                    continue;
                }
                if (!package.getRequired() && onlyRequired) {
                    continue;
                }

                for (auto &repo : package.getRepositories()) {
                    for (auto &file : repo.getFiles()) {
                        if (file.getStatus() != VersionCheck::FileStatus::Latest) {
                            result.push_back(file);
                        }
                    }
                }
            }
        }
    }
    return result;
}