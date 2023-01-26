//  To parse this JSON data, first install
//
//      json.hpp  https://github.com/nlohmann/json
//
//  Then include this file, and then do
//
//     BaseCategories data = nlohmann::json::parse(jsonString);

#pragma once

#include "json.hpp"

#include <optional>
#include <regex>
#include <stdexcept>

#ifndef NLOHMANN_OPT_HELPER
#define NLOHMANN_OPT_HELPER
namespace nlohmann {
    template<typename T>
    struct adl_serializer<std::shared_ptr<T>> {
        static void to_json(json &j, const std::shared_ptr<T> &opt) {
            if (!opt) j = nullptr;
            else
                j = *opt;
        }

        static std::shared_ptr<T> from_json(const json &j) {
            if (j.is_null()) return std::unique_ptr<T>();
            else
                return std::unique_ptr<T>(new T(j.get<T>()));
        }
    };
} // namespace nlohmann
#endif

namespace VersionCheck {
    using nlohmann::json;

#ifndef NLOHMANN_UNTYPED_VersionCheck_HELPERHELPER
#define NLOHMANN_UNTYPED_VersionCheck_HELPERHELPER
    inline json get_untyped(const json &j, const char *property) {
        if (j.find(property) != j.end()) {
            return j.at(property).get<json>();
        }
        return json();
    }

    inline json get_untyped(const json &j, std::string property) {
        return get_untyped(j, property.data());
    }
#endif

#ifndef NLOHMANN_OPTIONAL_VersionCheck_
#define NLOHMANN_OPTIONAL_VersionCheck_
    template<typename T>
    inline std::shared_ptr<T> get_optional(const json &j, const char *property) {
        if (j.find(property) != j.end()) {
            return j.at(property).get<std::shared_ptr<T>>();
        }
        return std::shared_ptr<T>();
    }

    template<typename T>
    inline std::shared_ptr<T> get_optional(const json &j, std::string property) {
        return get_optional<T>(j, property.data());
    }
#endif
    enum class FileStatus : int { Latest,
                                  Missing,
                                  Outdated,
                                  UnknownFile,
                                  Newer,
                                  Error };

    enum class RepoStatus : int { Latest,
                                  Dirty,
                                  Missing };

    enum class PackageStatus : int { Latest,
                                     Dirty,
                                     Missing };

    class AssetFile {
    public:
        AssetFile()          = default;
        virtual ~AssetFile() = default;

    private:
        std::string path;
        std::string sha1;

    public:
        const std::string &getPath() const { return path; }
        std::string &getMutablePath() { return path; }
        void setPath(const std::string &value) { this->path = value; }

        const std::string &getSha1() const { return sha1; }
        std::string &getMutableSha1() { return sha1; }
        void setSha1(const std::string &value) { this->sha1 = value; }
    };

    class Asset {
    public:
        Asset()          = default;
        virtual ~Asset() = default;

    private:
        int64_t id;
        std::string nodeId;
        std::string name;
        std::string contentType;
        std::string url;
        int64_t size;
        std::string createdAt;
        std::string updatedAt;
        std::string browserDownloadUrl;
        std::vector<AssetFile> files;

    public:
        const int64_t &getId() const { return id; }
        int64_t &getMutableId() { return id; }
        void setId(const int64_t &value) { this->id = value; }

        const std::string &getNodeId() const { return nodeId; }
        std::string &getMutableNodeId() { return nodeId; }
        void setNodeId(const std::string &value) { this->nodeId = value; }

        const std::string &getName() const { return name; }
        std::string &getMutableName() { return name; }
        void setName(const std::string &value) { this->name = value; }

        const std::string &getContentType() const { return contentType; }
        std::string &getMutableContentType() { return contentType; }
        void setContentType(const std::string &value) { this->contentType = value; }

        const std::string &getUrl() const { return url; }
        std::string &getMutableUrl() { return url; }
        void setUrl(const std::string &value) { this->url = value; }

        const int64_t &getSize() const { return size; }
        int64_t &getMutableSize() { return size; }
        void setSize(const int64_t &value) { this->size = value; }

        const std::string &getCreatedAt() const { return createdAt; }
        std::string &getMutableCreatedAt() { return createdAt; }
        void setCreatedAt(const std::string &value) { this->createdAt = value; }

        const std::string &getUpdatedAt() const { return updatedAt; }
        std::string &getMutableUpdatedAt() { return updatedAt; }
        void setUpdatedAt(const std::string &value) { this->updatedAt = value; }

        const std::string &getBrowserDownloadUrl() const { return browserDownloadUrl; }
        std::string &getMutableBrowserDownloadUrl() { return browserDownloadUrl; }
        void setBrowserDownloadUrl(const std::string &value) { this->browserDownloadUrl = value; }

        const std::vector<AssetFile> &getFiles() const { return files; }
        std::vector<AssetFile> &getMutableFiles() { return files; }
        void setFiles(const std::vector<AssetFile> &value) { this->files = value; }
    };

    class Release {
    public:
        Release()          = default;
        virtual ~Release() = default;

    private:
        int64_t id;
        std::string nodeId;
        std::string name;
        std::string tagName;
        bool prerelease;
        std::string targetCommitish;
        std::string createdAt;
        std::string publishedAt;

    public:
        const int64_t &getId() const { return id; }
        int64_t &getMutableId() { return id; }
        void setId(const int64_t &value) { this->id = value; }

        const std::string &getNodeId() const { return nodeId; }
        std::string &getMutableNodeId() { return nodeId; }
        void setNodeId(const std::string &value) { this->nodeId = value; }

        const std::string &getName() const { return name; }
        std::string &getMutableName() { return name; }
        void setName(const std::string &value) { this->name = value; }

        const std::string &getTagName() const { return tagName; }
        std::string &getMutableTagName() { return tagName; }
        void setTagName(const std::string &value) { this->tagName = value; }

        const bool &getPrerelease() const { return prerelease; }
        bool &getMutablePrerelease() { return prerelease; }
        void setPrerelease(const bool &value) { this->prerelease = value; }

        const std::string &getTargetCommitish() const { return targetCommitish; }
        std::string &getMutableTargetCommitish() { return targetCommitish; }
        void setTargetCommitish(const std::string &value) { this->targetCommitish = value; }

        const std::string &getCreatedAt() const { return createdAt; }
        std::string &getMutableCreatedAt() { return createdAt; }
        void setCreatedAt(const std::string &value) { this->createdAt = value; }

        const std::string &getPublishedAt() const { return publishedAt; }
        std::string &getMutablePublishedAt() { return publishedAt; }
        void setPublishedAt(const std::string &value) { this->publishedAt = value; }
    };

    class RepositoryFile {
    public:
        RepositoryFile()          = default;
        virtual ~RepositoryFile() = default;

    private:
        std::string path;
        std::string sha1;
        Asset asset;
        Release release;
        FileStatus status;

    public:
        const std::string &getPath() const { return path; }
        std::string &getMutablePath() { return path; }
        void setPath(const std::string &value) { this->path = value; }

        const std::string &getSha1() const { return sha1; }
        std::string &getMutableSha1() { return sha1; }
        void setSha1(const std::string &value) { this->sha1 = value; }

        const Asset &getAsset() const { return asset; }
        Asset &getMutableAsset() { return asset; }
        void setAsset(const Asset &value) { this->asset = value; }

        const Release &getRelease() const { return release; }
        Release &getMutableRelease() { return release; }
        void setRelease(const Release &value) { this->release = value; }

        const FileStatus &getStatus() const { return status; }
        FileStatus &getMutableStatus() { return status; }
        void setStatus(const FileStatus &value) { this->status = value; }
    };

    class Tag {
    public:
        Tag()          = default;
        virtual ~Tag() = default;

    private:
        std::string tagName;

    public:
        const std::string &getTagName() const { return tagName; }
        std::string &getMutableTagName() { return tagName; }
        void setTagName(const std::string &value) { this->tagName = value; }
    };

    class Repo {
    public:
        Repo()          = default;
        virtual ~Repo() = default;

    private:
        std::string owner;
        std::string repo;
        Tag tag;
        std::shared_ptr<bool> forceTag;

    public:
        const std::string &getOwner() const { return owner; }
        std::string &getMutableOwner() { return owner; }
        void setOwner(const std::string &value) { this->owner = value; }

        const std::string &getRepo() const { return repo; }
        std::string &getMutableRepo() { return repo; }
        void setRepo(const std::string &value) { this->repo = value; }

        const Tag &getTag() const { return tag; }
        Tag &getMutableTag() { return tag; }
        void setTag(const Tag &value) { this->tag = value; }

        std::shared_ptr<bool> getForceTag() const { return forceTag; }
        void setForceTag(std::shared_ptr<bool> value) { this->forceTag = value; }
    };

    class Repository {
    public:
        Repository()          = default;
        virtual ~Repository() = default;

    private:
        Repo repo;
        std::vector<RepositoryFile> files;
        RepoStatus status;

    public:
        const Repo &getRepo() const { return repo; }
        Repo &getMutableRepo() { return repo; }
        void setRepo(const Repo &value) { this->repo = value; }

        const std::vector<RepositoryFile> &getFiles() const { return files; }
        std::vector<RepositoryFile> &getMutableFiles() { return files; }
        void setFiles(const std::vector<RepositoryFile> &value) { this->files = value; }

        const RepoStatus &getStatus() const { return status; }
        RepoStatus &getMutableStatus() { return status; }
        void setStatus(const RepoStatus &value) { this->status = value; }

        size_t getFilesCount(bool onlyRequired) const {
            return files.size();
        }
    };

    class Package {
    public:
        Package()          = default;
        virtual ~Package() = default;

    private:
        std::string id;
        std::string name;
        std::string description;
        std::vector<Repository> repositories;
        bool required;
        bool preSelected;
        PackageStatus status;
        std::string lastUpdateDate;

        bool isSelected = false;

    public:
        const std::string &getId() const { return id; }
        std::string &getMutableId() { return id; }
        void setId(const std::string &value) { this->id = value; }

        const std::string &getName() const { return name; }
        std::string &getMutableName() { return name; }
        void setName(const std::string &value) { this->name = value; }

        const std::string &getDescription() const { return description; }
        std::string &getMutableDescription() { return description; }
        void setDescription(const std::string &value) { this->description = value; }

        const std::vector<Repository> &getRepositories() const { return repositories; }
        std::vector<Repository> &getMutableRepositories() { return repositories; }
        void setRepositories(const std::vector<Repository> &value) { this->repositories = value; }

        const bool &getRequired() const { return required; }
        bool &getMutableRequired() { return required; }
        void setRequired(const bool &value) { this->required = value; }

        const bool &getPreSelected() const { return preSelected; }
        bool &getMutablePreSelected() { return preSelected; }
        void setPreSelected(const bool &value) { this->preSelected = value; }

        const PackageStatus &getStatus() const { return status; }
        PackageStatus &getMutableStatus() { return status; }
        void setStatus(const PackageStatus &value) { this->status = value; }

        const std::string &getLastUpdateDate() const { return lastUpdateDate; }
        std::string &getMutableLastUpdateDate() { return lastUpdateDate; }
        void setLastUpdateDate(const std::string &value) { this->lastUpdateDate = value; }

        const bool &getIsSelected() const { return this->isSelected; }
        void toggleSelect() {
            if (!required) this->isSelected = !this->isSelected;
        }
        void setSelected(const bool &value) {
            if (!required) this->isSelected = value;
        }

        size_t getFilesCount(bool onlyRequired) const {
            size_t res = 0;
            for (auto &repo : getRepositories()) {
                res += repo.getFilesCount(onlyRequired);
            }
            return res;
        }
    };

    class Category {
    public:
        Category()          = default;
        virtual ~Category() = default;

    private:
        std::string id;
        std::string name;
        std::vector<Package> packages;

    public:
        const std::string &getId() const { return id; }
        std::string &getMutableId() { return id; }
        void setId(const std::string &value) { this->id = value; }

        const std::string &getName() const { return name; }
        std::string &getMutableName() { return name; }
        void setName(const std::string &value) { this->name = value; }

        const std::vector<Package> &getPackages() const { return packages; }
        std::vector<Package> &getMutablePackages() { return packages; }
        void setPackages(const std::vector<Package> &value) { this->packages = value; }

        size_t getFilesCount(bool onlyRequired) const {
            size_t res = 0;
            for (auto &package : getPackages()) {
                res += package.getFilesCount(onlyRequired);
            }
            return res;
        }

        const size_t getPackagesCount(bool required) const {
            if (!required) {
                return packages.size();
            }
            size_t result = 0;
            for (auto &cur : packages) {
                if (cur.getRequired()) {
                    result++;
                }
            }
            return result;
        }
    };

    class BaseCategory {
    public:
        BaseCategory()          = default;
        virtual ~BaseCategory() = default;

    private:
        std::string id;
        std::string name;
        std::vector<Category> category;

    public:
        const std::string &getId() const { return id; }
        std::string &getMutableId() { return id; }
        void setId(const std::string &value) { this->id = value; }

        const std::string &getName() const { return name; }
        std::string &getMutableName() { return name; }
        void setName(const std::string &value) { this->name = value; }

        const std::vector<Category> &getCategory() const { return category; }
        std::vector<Category> &getMutableCategory() { return category; }
        void setCategory(const std::vector<Category> &value) { this->category = value; }

        size_t getFilesCount(bool onlyRequired) const {
            size_t res = 0;
            for (auto &cat : category) {
                res += cat.getFilesCount(onlyRequired);
            }
            return res;
        }

        const size_t getPackagesCount(bool required) const {
            size_t result = 0;
            for (auto &cur : category) {
                result += cur.getPackagesCount(required);
            }
            return result;
        }
    };

    using VersionInfo = std::vector<BaseCategory>;
} // namespace VersionCheck

namespace VersionCheck {
    void from_json(const json &j, AssetFile &x);

    void from_json(const json &j, Asset &x);

    void from_json(const json &j, Release &x);

    void from_json(const json &j, RepositoryFile &x);

    void from_json(const json &j, Tag &x);

    void from_json(const json &j, Repo &x);

    void from_json(const json &j, Repository &x);

    void from_json(const json &j, Package &x);

    void from_json(const json &j, Category &x);

    void from_json(const json &j, BaseCategory &x);

    inline void from_json(const json &j, AssetFile &x) {
        x.setPath(j.at("path").get<std::string>());
        x.setSha1(j.at("sha1").get<std::string>());
    }

    inline void from_json(const json &j, Asset &x) {
        x.setId(j.at("id").get<int64_t>());
        x.setNodeId(j.at("node_id").get<std::string>());
        x.setName(j.at("name").get<std::string>());
        x.setContentType(j.at("content_type").get<std::string>());
        x.setUrl(j.at("url").get<std::string>());
        x.setSize(j.at("size").get<int64_t>());
        x.setCreatedAt(j.at("created_at").get<std::string>());
        x.setUpdatedAt(j.at("updated_at").get<std::string>());
        x.setBrowserDownloadUrl(j.at("browser_download_url").get<std::string>());
        x.setFiles(j.at("files").get<std::vector<AssetFile>>());
    }

    inline void from_json(const json &j, Release &x) {
        x.setId(j.at("id").get<int64_t>());
        x.setNodeId(j.at("node_id").get<std::string>());
        x.setName(j.at("name").get<std::string>());
        x.setTagName(j.at("tag_name").get<std::string>());
        x.setPrerelease(j.at("prerelease").get<bool>());
        x.setTargetCommitish(j.at("target_commitish").get<std::string>());
        x.setCreatedAt(j.at("created_at").get<std::string>());
        x.setPublishedAt(j.at("published_at").get<std::string>());
    }

    inline void from_json(const json &j, RepositoryFile &x) {
        x.setPath(j.at("path").get<std::string>());
        x.setSha1(j.at("sha1").get<std::string>());
        x.setAsset(j.at("asset").get<Asset>());
        x.setRelease(j.at("release").get<Release>());
    }

    inline void from_json(const json &j, Tag &x) {
        x.setTagName(j.at("tag_name").get<std::string>());
    }

    inline void from_json(const json &j, Repo &x) {
        x.setOwner(j.at("owner").get<std::string>());
        x.setRepo(j.at("repo").get<std::string>());
        x.setTag(j.at("tag").get<Tag>());
        x.setForceTag(get_optional<bool>(j, "forceTag"));
    }

    inline void from_json(const json &j, Repository &x) {
        x.setRepo(j.at("repo").get<Repo>());
        x.setFiles(j.at("files").get<std::vector<RepositoryFile>>());
    }

    inline void from_json(const json &j, Package &x) {
        x.setId(j.at("id").get<std::string>());
        x.setName(j.at("name").get<std::string>());
        x.setDescription(j.at("description").get<std::string>());
        x.setRepositories(j.at("repositories").get<std::vector<Repository>>());
        x.setRequired(j.at("required").get<bool>());
        x.setPreSelected(j.at("preSelected").get<bool>());
    }

    inline void from_json(const json &j, Category &x) {
        x.setId(j.at("id").get<std::string>());
        x.setName(j.at("name").get<std::string>());
        x.setPackages(j.at("packages").get<std::vector<Package>>());
    }

    inline void from_json(const json &j, BaseCategory &x) {
        x.setId(j.at("id").get<std::string>());
        x.setName(j.at("name").get<std::string>());
        x.setCategory(j.at("category").get<std::vector<Category>>());
    }

} // namespace VersionCheck
