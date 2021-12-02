#include "OneDriveDirectory.hpp"
#include "request/Request.hpp"
#include "OneDriveCloud.hpp"
#include "OneDriveFile.hpp"
#include <filesystem>
#include <vector>

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::onedrive;
using json = nlohmann::json;
namespace fs = std::filesystem;

std::vector<std::shared_ptr<Resource>> OneDriveDirectory::list_resources() const {
    std::vector<std::shared_ptr<Resource>> resource_list;
    try {
        const json response_json = m_request->GET(this->apiResourcePath(this->path(), true))
                ->accept(Request::MIMETYPE_JSON)
                ->send().json();
        for (const auto &value: response_json.at("value")) {
            resource_list.push_back(this->parseDriveItem(value));
        }
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
    return resource_list;
}

std::shared_ptr<Directory> OneDriveDirectory::get_directory(const std::string &path) const {
    std::shared_ptr<OneDriveDirectory> directory;
    const auto resourcePath = this->newResourcePath(path);
    try {
        if (resourcePath == "/") {
            // if it's the root we don't need to run a query
            directory = std::make_shared<OneDriveDirectory>(m_base_url, "/", m_request, "");
        } else {
            json response_json = m_request->GET(this->apiResourcePath(resourcePath, false))
                    ->accept(Request::MIMETYPE_JSON)
                    ->send().json();
            directory = std::dynamic_pointer_cast<OneDriveDirectory>(this->parseDriveItem(response_json, "folder"));
        }

    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return directory;
}

void OneDriveDirectory::remove() {
    try {
        if (this->path() != "/") {
            m_request->DELETE(m_base_url + ":" + this->path())->send();
        } else {
            throw PermissionDenied("deleting the root folder is not allowed");
        }
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
}

std::shared_ptr<Directory> OneDriveDirectory::create_directory(const std::string &path) const {
    std::shared_ptr<OneDriveDirectory> newDirectory;
    const auto resourcePath = fs::path(this->newResourcePath(path));
    const std::string newResourceBasePath = resourcePath.parent_path().generic_string();
    const std::string newDirectoryName = resourcePath.filename().string();
    try {
        const json response_json = m_request->POST(this->apiResourcePath(newResourceBasePath, true))
                ->accept(Request::MIMETYPE_JSON)
                ->send_json({
                        {"name", newDirectoryName},
                        {"folder", json::object()},
                        {"@microsoft.graph.conflictBehavior", "fail"}
                }).json();
        newDirectory = std::dynamic_pointer_cast<OneDriveDirectory>(this->parseDriveItem(response_json, "folder"));
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), resourcePath.generic_string());
    }
    return newDirectory;
}

std::shared_ptr<File> OneDriveDirectory::create_file(const std::string &path) const {
    std::shared_ptr<OneDriveFile> newFile;
    const std::string resourcePath = this->newResourcePath(path);
    try {
        const json response_json = m_request->PUT(m_base_url + ":" + resourcePath + ":/content")
                ->content_type(Request::MIMETYPE_BINARY)
                ->accept(Request::MIMETYPE_JSON)
                ->query_param("@microsoft.graph.conflictBehavior", "fail")
                ->send("").json();
        newFile = std::dynamic_pointer_cast<OneDriveFile>(this->parseDriveItem(response_json, "file"));
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return newFile;
}

std::shared_ptr<File> OneDriveDirectory::get_file(const std::string &path) const {
    std::shared_ptr<OneDriveFile> file;
    const std::string resourcePath = this->newResourcePath(path);
    try {
        const json response_json = m_request->GET(m_base_url + ":" + resourcePath)
                ->accept(Request::MIMETYPE_JSON)
                ->send().json();
        file = std::dynamic_pointer_cast<OneDriveFile>(this->parseDriveItem(response_json, "file"));
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return file;
}

std::shared_ptr<Resource>
OneDriveDirectory::parseDriveItem(const json &value, const std::string &expectedType) const {
    std::shared_ptr<Resource> resource;
    const std::string name = value.at("name");
    // check if the returned item is the root item
    if (value.find("root") != value.end()) {
        resource = std::make_shared<OneDriveDirectory>(m_base_url, "/", m_request, "");
    } else {
        const std::string rawResourcePath = value.at("parentReference").at("path");
        const auto splitPosition = rawResourcePath.find_first_of(':') + 1;
        const std::string resourcePath =
                rawResourcePath.substr(splitPosition, rawResourcePath.size() - splitPosition) + "/";
        if (value.find("file") != value.end() && (expectedType.empty() || expectedType == "file")) {
            const std::string etag = value["eTag"];
            resource = std::make_shared<OneDriveFile>(m_base_url, resourcePath + name, m_request, name,
                                                      etag);
        } else if (value.find("folder") != value.end() && (expectedType.empty() || expectedType == "folder")) {
            resource = std::make_shared<OneDriveDirectory>(m_base_url, resourcePath + name, m_request,
                                                           name);
        } else {
            throw Resource::NoSuchResource("unexpected resource type");
        }
    }
    return resource;
}

std::string OneDriveDirectory::apiResourcePath(const std::string &path, bool children) const {
    std::string out = m_base_url;
    if (path == "/") {
        if (children) {
            out += "/children";
        }
    } else {
        out += ":" + path;
        if (children) {
            out += ":/children";
        }
    }
    return out;
}

std::string OneDriveDirectory::newResourcePath(const std::string &path) const {
    std::string normalizedPath = (fs::path(this->path()) / path).lexically_normal().generic_string();
    // remove trailing slashes because onedrive won't accept them
    while (normalizedPath.size() > 1 && normalizedPath.back() == '/') {
        normalizedPath = normalizedPath.erase(normalizedPath.size() - 1);
    }
    return normalizedPath;
}
