// clang-format off
//  To parse this JSON data, first install
//
//      json.hpp  https://github.com/nlohmann/json
//
//  Then include this file, and then do
//
//     VersionInfo data = nlohmann::json::parse(jsonString);

#pragma once

#include "json.hpp"

#include <optional>
#include <stdexcept>
#include <regex>

namespace VersionByHash {
    using nlohmann::json;

    inline json get_untyped(const json & j, const char * property) {
        if (j.find(property) != j.end()) {
            return j.at(property).get<json>();
        }
        return json();
    }

    inline json get_untyped(const json & j, std::string property) {
        return get_untyped(j, property.data());
    }

    class File {
    public:
        File() = default;
        virtual ~File() = default;

    private:
        std::string path;
        std::string sha1;

    public:
        const std::string & getPath() const { return path; }
        std::string & getMutablePath() { return path; }
        void setPath(const std::string & value) { this->path = value; }

        const std::string & getSha1() const { return sha1; }
        std::string & getMutableSha1() { return sha1; }
        void setSha1(const std::string & value) { this->sha1 = value; }
    };

    class Assets {
    public:
        Assets() = default;
        virtual ~Assets() = default;

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
        std::vector<File> files;

    public:
        const int64_t & getId() const { return id; }
        int64_t & getMutableId() { return id; }
        void setId(const int64_t & value) { this->id = value; }

        const std::string & getNodeId() const { return nodeId; }
        std::string & getMutableNodeId() { return nodeId; }
        void setNodeId(const std::string & value) { this->nodeId = value; }

        const std::string & getName() const { return name; }
        std::string & getMutableName() { return name; }
        void setName(const std::string & value) { this->name = value; }

        const std::string & getContentType() const { return contentType; }
        std::string & getMutableContentType() { return contentType; }
        void setContentType(const std::string & value) { this->contentType = value; }

        const std::string & getUrl() const { return url; }
        std::string & getMutableUrl() { return url; }
        void setUrl(const std::string & value) { this->url = value; }

        const int64_t & getSize() const { return size; }
        int64_t & getMutableSize() { return size; }
        void setSize(const int64_t & value) { this->size = value; }

        const std::string & getCreatedAt() const { return createdAt; }
        std::string & getMutableCreatedAt() { return createdAt; }
        void setCreatedAt(const std::string & value) { this->createdAt = value; }

        const std::string & getUpdatedAt() const { return updatedAt; }
        std::string & getMutableUpdatedAt() { return updatedAt; }
        void setUpdatedAt(const std::string & value) { this->updatedAt = value; }

        const std::string & getBrowserDownloadUrl() const { return browserDownloadUrl; }
        std::string & getMutableBrowserDownloadUrl() { return browserDownloadUrl; }
        void setBrowserDownloadUrl(const std::string & value) { this->browserDownloadUrl = value; }

        const std::vector<File> & getFiles() const { return files; }
        std::vector<File> & getMutableFiles() { return files; }
        void setFiles(const std::vector<File> & value) { this->files = value; }
    };

    class Release {
    public:
        Release() = default;
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
        const int64_t & getId() const { return id; }
        int64_t & getMutableId() { return id; }
        void setId(const int64_t & value) { this->id = value; }

        const std::string & getNodeId() const { return nodeId; }
        std::string & getMutableNodeId() { return nodeId; }
        void setNodeId(const std::string & value) { this->nodeId = value; }

        const std::string & getName() const { return name; }
        std::string & getMutableName() { return name; }
        void setName(const std::string & value) { this->name = value; }

        const std::string & getTagName() const { return tagName; }
        std::string & getMutableTagName() { return tagName; }
        void setTagName(const std::string & value) { this->tagName = value; }

        const bool & getPrerelease() const { return prerelease; }
        bool & getMutablePrerelease() { return prerelease; }
        void setPrerelease(const bool & value) { this->prerelease = value; }

        const std::string & getTargetCommitish() const { return targetCommitish; }
        std::string & getMutableTargetCommitish() { return targetCommitish; }
        void setTargetCommitish(const std::string & value) { this->targetCommitish = value; }

        const std::string & getCreatedAt() const { return createdAt; }
        std::string & getMutableCreatedAt() { return createdAt; }
        void setCreatedAt(const std::string & value) { this->createdAt = value; }

        const std::string & getPublishedAt() const { return publishedAt; }
        std::string & getMutablePublishedAt() { return publishedAt; }
        void setPublishedAt(const std::string & value) { this->publishedAt = value; }
    };

    class VersionInfo {
    public:
        VersionInfo() = default;
        virtual ~VersionInfo() = default;

    private:
        std::string repo;
        Assets assets;
        Release release;

    public:
        const std::string & getRepo() const { return repo; }
        std::string & getMutableRepo() { return repo; }
        void setRepo(const std::string & value) { this->repo = value; }

        const Assets & getAssets() const { return assets; }
        Assets & getMutableAssets() { return assets; }
        void setAssets(const Assets & value) { this->assets = value; }

        const Release & getRelease() const { return release; }
        Release & getMutableRelease() { return release; }
        void setRelease(const Release & value) { this->release = value; }
    };
}

namespace nlohmann {
    void from_json(const json & j, VersionByHash::File & x);
    void to_json(json & j, const VersionByHash::File & x);

    void from_json(const json & j, VersionByHash::Assets & x);
    void to_json(json & j, const VersionByHash::Assets & x);

    void from_json(const json & j, VersionByHash::Release & x);
    void to_json(json & j, const VersionByHash::Release & x);

    void from_json(const json & j, VersionByHash::VersionInfo & x);
    void to_json(json & j, const VersionByHash::VersionInfo & x);

    inline void from_json(const json & j, VersionByHash::File& x) {
        x.setPath(j.at("path").get<std::string>());
        x.setSha1(j.at("sha1").get<std::string>());
    }

    inline void to_json(json & j, const VersionByHash::File & x) {
        j = json::object();
        j["path"] = x.getPath();
        j["sha1"] = x.getSha1();
    }

    inline void from_json(const json & j, VersionByHash::Assets& x) {
        x.setId(j.at("id").get<int64_t>());
        x.setNodeId(j.at("node_id").get<std::string>());
        x.setName(j.at("name").get<std::string>());
        x.setContentType(j.at("content_type").get<std::string>());
        x.setUrl(j.at("url").get<std::string>());
        x.setSize(j.at("size").get<int64_t>());
        x.setCreatedAt(j.at("created_at").get<std::string>());
        x.setUpdatedAt(j.at("updated_at").get<std::string>());
        x.setBrowserDownloadUrl(j.at("browser_download_url").get<std::string>());
        x.setFiles(j.at("files").get<std::vector<VersionByHash::File>>());
    }

    inline void to_json(json & j, const VersionByHash::Assets & x) {
        j = json::object();
        j["id"] = x.getId();
        j["node_id"] = x.getNodeId();
        j["name"] = x.getName();
        j["content_type"] = x.getContentType();
        j["url"] = x.getUrl();
        j["size"] = x.getSize();
        j["created_at"] = x.getCreatedAt();
        j["updated_at"] = x.getUpdatedAt();
        j["browser_download_url"] = x.getBrowserDownloadUrl();
        j["files"] = x.getFiles();
    }

    inline void from_json(const json & j, VersionByHash::Release& x) {
        x.setId(j.at("id").get<int64_t>());
        x.setNodeId(j.at("node_id").get<std::string>());
        x.setName(j.at("name").get<std::string>());
        x.setTagName(j.at("tag_name").get<std::string>());
        x.setPrerelease(j.at("prerelease").get<bool>());
        x.setTargetCommitish(j.at("target_commitish").get<std::string>());
        x.setCreatedAt(j.at("created_at").get<std::string>());
        x.setPublishedAt(j.at("published_at").get<std::string>());
    }

    inline void to_json(json & j, const VersionByHash::Release & x) {
        j = json::object();
        j["id"] = x.getId();
        j["node_id"] = x.getNodeId();
        j["name"] = x.getName();
        j["tag_name"] = x.getTagName();
        j["prerelease"] = x.getPrerelease();
        j["target_commitish"] = x.getTargetCommitish();
        j["created_at"] = x.getCreatedAt();
        j["published_at"] = x.getPublishedAt();
    }

    inline void from_json(const json & j, VersionByHash::VersionInfo& x) {
        x.setRepo(j.at("repo").get<std::string>());
        x.setAssets(j.at("assets").get<VersionByHash::Assets>());
        x.setRelease(j.at("release").get<VersionByHash::Release>());
    }

    inline void to_json(json & j, const VersionByHash::VersionInfo & x) {
        j = json::object();
        j["repo"] = x.getRepo();
        j["assets"] = x.getAssets();
        j["release"] = x.getRelease();
    }
}
