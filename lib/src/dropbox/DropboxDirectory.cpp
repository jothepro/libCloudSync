#include "DropboxDirectory.hpp"
#include "request/Request.hpp"
#include "request/Response.hpp"
#include "DropboxCloud.hpp"
#include "DropboxFile.hpp"
#include <filesystem>
#include <vector>

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::dropbox;
using json = nlohmann::json;
namespace fs = std::filesystem;

std::vector<std::shared_ptr<Resource>> DropboxDirectory::list_resources() const {
    const auto resourcePath = DropboxDirectory::parsePath(this->path());
    std::vector<std::shared_ptr<Resource>> resources;
    try {
        const json response_json = m_request->POST("https://api.dropboxapi.com/2/files/list_folder")
                ->accept(Request::MIMETYPE_JSON)
                ->send_json({
                        {"path",      resourcePath},
                        {"recursive", false}
                }).json();
        for (const auto &entry: response_json.at("entries")) {
            resources.push_back(this->parseEntry(entry));
        }
        // the following code takes care of paging
        bool hasMore = response_json.at("has_more");
        std::string cursor = response_json.at("cursor");
        while(hasMore) {
            const json continue_response_json = m_request->POST(
                            "https://api.dropboxapi.com/2/files/list_folder/continue")
                    ->accept(Request::MIMETYPE_JSON)
                    ->send_json({
                            {"cursor", cursor}
                    }).json();
            hasMore = continue_response_json.at("has_more");
            cursor = continue_response_json.at("cursor");
            for (const auto &entry: continue_response_json.at("entries")) {
                resources.push_back(this->parseEntry(entry));
            }
        }
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return resources;
}

std::shared_ptr<Directory> DropboxDirectory::get_directory(const std::string &path) const {
    const auto resourcePath = DropboxDirectory::parsePath(this->path(), path);
    std::shared_ptr<DropboxDirectory> directory;
    // get_metadata is not supported for the root folder
    if (!resourcePath.empty()) {
        try {
            const json response_json = m_request->POST("https://api.dropboxapi.com/2/files/get_metadata")
                    ->accept(Request::MIMETYPE_JSON)
                    ->send_json({
                            {"path", resourcePath}
                    }).json();
            directory = std::dynamic_pointer_cast<DropboxDirectory>(this->parseEntry(response_json, "folder"));

        } catch (...) {
            DropboxCloud::handleExceptions(std::current_exception(), resourcePath);
        }
    } else {
        directory = std::make_shared<DropboxDirectory>("/", m_request, "");
    }
    return directory;
}

void DropboxDirectory::remove() {
    const auto resourcePath = DropboxDirectory::parsePath(this->path());
    if (resourcePath.empty()) {
        throw PermissionDenied("deleting the root folder is not allowed");
    }
    try {
        m_request->POST("https://api.dropboxapi.com/2/files/delete_v2")
                ->send_json({{"path", resourcePath}});
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), resourcePath);
    }
}

std::shared_ptr<Directory> DropboxDirectory::create_directory(const std::string &path) const {
    const auto resourcePath = DropboxDirectory::parsePath(this->path(), path);
    std::shared_ptr<Directory> directory;
    try {
        const json response_json = m_request->POST("https://api.dropboxapi.com/2/files/create_folder_v2")
                ->accept(Request::MIMETYPE_JSON)
                ->send_json({
                        {"path", resourcePath}
                }).json();
        directory = std::dynamic_pointer_cast<DropboxDirectory>(
                        this->parseEntry(response_json.at("metadata"), "folder"));
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return directory;
}

std::shared_ptr<File> DropboxDirectory::create_file(const std::string &path) const {
    const auto resourcePath = DropboxDirectory::parsePath(this->path(), path);
    std::shared_ptr<File> file;
    try {
        const json response_json = m_request->POST("https://content.dropboxapi.com/2/files/upload")
                ->content_type(Request::MIMETYPE_BINARY)
                ->accept(Request::MIMETYPE_JSON)
                ->query_param("arg", json{
                        {"path", resourcePath}
                }.dump())->send().json();
        file = std::dynamic_pointer_cast<DropboxFile>(this->parseEntry(response_json, "file"));
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return file;
}

std::shared_ptr<File> DropboxDirectory::get_file(const std::string &path) const {
    const auto resourcePath = DropboxDirectory::parsePath(this->path(), path);
    std::shared_ptr<DropboxFile> file;
    try {
        json response_json = m_request->POST("https://api.dropboxapi.com/2/files/get_metadata")
                ->accept(Request::MIMETYPE_JSON)
                ->send_json({
                        {"path", resourcePath}
                }).json();
        file = std::dynamic_pointer_cast<DropboxFile>(this->parseEntry(response_json, "file"));
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return file;
}

std::shared_ptr<Resource>
DropboxDirectory::parseEntry(const json &entry, const std::string &resourceTypeFallback) const {
    std::shared_ptr<Resource> resource;
    std::string resourceType;
    const std::string name = entry.at("name");
    std::string path = entry.at("path_display");
    if (entry.find(".tag") != entry.end()) {
        resourceType = entry[".tag"];
    } else {
        if (!resourceTypeFallback.empty()) {
            resourceType = resourceTypeFallback;
        } else {
            throw Cloud::CommunicationError("unknown resource type");
        }
    }
    if (!resourceTypeFallback.empty() && resourceType != resourceTypeFallback) {
        throw NoSuchResource(path);
    }
    if (resourceType == "folder") {
        resource = std::make_shared<DropboxDirectory>(path, m_request, name);
    } else if (resourceType == "file") {
        resource = std::make_shared<DropboxFile>(path, m_request, name, entry.at("rev"));
    }
    return resource;
}

std::string DropboxDirectory::parsePath(const std::string &path, const std::string &path2) {
    std::string parsedPath = (fs::path(path) / path2).lexically_normal().generic_string();
    // remove trailing slashes because dropbox won't accept them
    while (!parsedPath.empty() && parsedPath.back() == '/') {
        parsedPath = parsedPath.erase(parsedPath.size() - 1);
    }
    return parsedPath;
}
