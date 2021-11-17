#include "DropboxDirectory.hpp"
#include "request/Request.hpp"
#include "request/Response.hpp"
#include "DropboxCloud.hpp"
#include "DropboxFile.hpp"
#include <filesystem>
#include <vector>

using json = nlohmann::json;
using namespace CloudSync::request;
using P = Request::ParameterType;

namespace CloudSync::dropbox {
std::vector<std::shared_ptr<Resource>> DropboxDirectory::ls() const {
    // TODO implement paging logic when list is split in multiple responses
    const auto resourcePath = DropboxDirectory::parsePath(this->path);
    std::vector<std::shared_ptr<Resource>> resources;
    try {
        json responseJson = this->request
                                ->POST(
                                    "https://api.dropboxapi.com/2/files/list_folder",
                                    {{P::HEADERS, {{"Content-Type", Request::MIMETYPE_JSON}}}},
                                    json{{"path", resourcePath}, {"recursive", false}}.dump())
                                .json();
        for (const auto &entry : responseJson.at("entries")) {
            resources.push_back(this->parseEntry(entry));
        }
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return resources;
}
std::shared_ptr<Directory> DropboxDirectory::cd(const std::string &path) const {
    const auto resourcePath = DropboxDirectory::parsePath(this->path + "/" + path);
    std::shared_ptr<DropboxDirectory> directory;
    // get_metadata is not supported for the root folder
    if (resourcePath != "") {
        try {
            json responseJson = this->request
                                    ->POST(
                                        "https://api.dropboxapi.com/2/files/get_metadata",
                                        {{P::HEADERS, {{"Content-Type", Request::MIMETYPE_JSON}}}},
                                        json{{"path", resourcePath}}.dump())
                                    .json();
            directory = std::dynamic_pointer_cast<DropboxDirectory>(this->parseEntry(responseJson));

        } catch (...) {
            DropboxCloud::handleExceptions(std::current_exception(), resourcePath + "/");
        }
    } else {
        directory = std::make_shared<DropboxDirectory>("/", this->request, "");
    }
    return directory;
}

void DropboxDirectory::rmdir() const {
    const auto resourcePath = DropboxDirectory::parsePath(this->path);
    if (resourcePath == "") {
        throw PermissionDenied("deleting the root folder is not allowed");
    }
    try {
        this->request->POST(
            "https://api.dropboxapi.com/2/files/delete_v2",
            {{P::HEADERS, {{"Content-Type", Request::MIMETYPE_JSON}}}},
            json{{"path", resourcePath}}.dump());
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), resourcePath);
    }
}

std::shared_ptr<Directory> DropboxDirectory::mkdir(const std::string &path) const {
    const auto resourcePath = DropboxDirectory::parsePath(this->path + "/" + path);
    std::shared_ptr<DropboxDirectory> directory;
    try {
        json responseJson = this->request
                                ->POST(
                                    "https://api.dropboxapi.com/2/files/create_folder_v2",
                                    {{P::HEADERS, {{"Content-Type", Request::MIMETYPE_JSON}}}},
                                    json{{"path", resourcePath}}.dump())
                                .json();
        directory =
            std::dynamic_pointer_cast<DropboxDirectory>(this->parseEntry(responseJson.at("metadata"), "folder"));
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return directory;
}
std::shared_ptr<File> DropboxDirectory::touch(const std::string &path) const {
    const auto resourcePath = DropboxDirectory::parsePath(this->path + "/" + path);
    std::shared_ptr<DropboxFile> file;
    try {
        const auto responseJson = this->request
                                      ->POST(
                                          "https://content.dropboxapi.com/2/files/upload",
                                          {{P::HEADERS, {{"Content-Type", Request::MIMETYPE_BINARY}}},
                                           {P::QUERY_PARAMS, {{"arg", json{{"path", resourcePath}}.dump()}}}},
                                          "")
                                      .json();
        file = std::dynamic_pointer_cast<DropboxFile>(this->parseEntry(responseJson, "file"));
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return file;
}
std::shared_ptr<File> DropboxDirectory::file(const std::string &path) const {
    const auto resourcePath = DropboxDirectory::parsePath(this->path + "/" + path);
    std::shared_ptr<DropboxFile> file;
    try {
        json responseJson = this->request
                                ->POST(
                                    "https://api.dropboxapi.com/2/files/get_metadata",
                                    {{P::HEADERS, {{"Content-Type", Request::MIMETYPE_JSON}}}},
                                    json{{"path", resourcePath}}.dump())
                                .json();
        file = std::dynamic_pointer_cast<DropboxFile>(this->parseEntry(responseJson, "file"));
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
        if (resourceTypeFallback != "") {
            resourceType = resourceTypeFallback;
        } else {
            throw Cloud::CommunicationError("unknown resource type");
        }
    }
    if (resourceTypeFallback != "" && resourceType != resourceTypeFallback) {
        throw NoSuchFileOrDirectory(path);
    }
    if (resourceType == "folder") {
        resource = std::make_shared<DropboxDirectory>(path, this->request, name);
    } else if (resourceType == "file") {
        resource = std::make_shared<DropboxFile>(path, this->request, name, entry.at("rev"));
    }
    return resource;
}

std::string DropboxDirectory::parsePath(const std::string &path) {
    std::string parsedPath = std::filesystem::path(path).lexically_normal().generic_string();
    // remove trailing slashes because dropbox won't accept them
    while (!parsedPath.empty() && parsedPath.back() == '/') {
        parsedPath = parsedPath.erase(parsedPath.size() - 1);
    }
    return parsedPath;
}

} // namespace CloudSync::dropbox
