#include "OneDriveDirectory.hpp"
#include "request/Request.hpp"
#include "OneDriveCloud.hpp"
#include "OneDriveFile.hpp"
#include <filesystem>
#include <sstream>
#include <vector>

using json = nlohmann::json;
using namespace CloudSync::request;
using P = Request::ParameterType;
namespace fs = std::filesystem;

namespace CloudSync::onedrive {
    std::vector<std::shared_ptr<Resource>> OneDriveDirectory::ls() const {
        std::vector<std::shared_ptr<Resource>> resourceList;
        try {
            json responseJson = this->request->GET(this->apiResourcePath(this->path(), true)).json();
            for (const auto &value: responseJson.at("value")) {
                resourceList.push_back(this->parseDriveItem(value));
            }
        } catch (...) {
            OneDriveCloud::handleExceptions(std::current_exception(), this->path());
        }
        return resourceList;
    }

    std::shared_ptr<Directory> OneDriveDirectory::cd(const std::string &path) const {
        std::shared_ptr<OneDriveDirectory> directory;
        const auto resourcePath = this->newResourcePath(path);
        try {
            if (resourcePath == "/") {
                // if it's the root we don't need to run a query
                directory = std::make_shared<OneDriveDirectory>(this->_baseUrl, "/", this->request, "");
            } else {
                json responseJson = this->request->GET(this->apiResourcePath(resourcePath, false)).json();
                directory = std::dynamic_pointer_cast<OneDriveDirectory>(this->parseDriveItem(responseJson, "folder"));
            }

        } catch (...) {
            OneDriveCloud::handleExceptions(std::current_exception(), resourcePath);
        }
        return directory;
    }

    void OneDriveDirectory::rmdir() const {
        try {
            if (this->path() != "/") {
                this->request->DELETE(this->_baseUrl + ":" + this->path());
            } else {
                throw PermissionDenied("deleting the root folder is not allowed");
            }
        } catch (...) {
            OneDriveCloud::handleExceptions(std::current_exception(), this->path());
        }
    }

    std::shared_ptr<Directory> OneDriveDirectory::mkdir(const std::string &path) const {
        std::shared_ptr<OneDriveDirectory> newDirectory;
        const auto resourcePath = fs::path(this->newResourcePath(path));
        const std::string newResourceBasePath = resourcePath.parent_path().generic_string();
        const std::string newDirectoryName = resourcePath.filename().string();
        try {
            const auto responseJson = this->request
                    ->POST(
                            this->apiResourcePath(newResourceBasePath, true),
                            {{P::HEADERS, {{"Content-Type", Request::MIMETYPE_JSON}}}},
                            json{{"name",   newDirectoryName},
                                 {"folder", json::object()}}.dump())
                    .json();
            newDirectory = std::dynamic_pointer_cast<OneDriveDirectory>(this->parseDriveItem(responseJson, "folder"));
        } catch (...) {
            OneDriveCloud::handleExceptions(std::current_exception(), resourcePath.generic_string());
        }
        return newDirectory;
    }

    std::shared_ptr<File> OneDriveDirectory::touch(const std::string &path) const {
        std::shared_ptr<OneDriveFile> newFile;
        const std::string resourcePath = this->newResourcePath(path);
        try {
            json responseJson = this->request->PUT(
                this->_baseUrl + ":" + resourcePath + ":/content",
                {
                    {
                        P::HEADERS, {
                            {"Content-Type", Request::MIMETYPE_BINARY}
                        }
                    }
                },
                ""
            ).json();
            newFile = std::dynamic_pointer_cast<OneDriveFile>(this->parseDriveItem(responseJson, "file"));
        } catch (...) {
            OneDriveCloud::handleExceptions(std::current_exception(), resourcePath);
        }
        return newFile;
    }

    std::shared_ptr<File> OneDriveDirectory::file(const std::string &path) const {
        std::shared_ptr<OneDriveFile> file;
        const std::string resourcePath = this->newResourcePath(path);
        try {
            json responseJson = this->request->GET(this->_baseUrl + ":" + resourcePath).json();
            file = std::dynamic_pointer_cast<OneDriveFile>(this->parseDriveItem(responseJson, "file"));
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
            resource = std::make_shared<OneDriveDirectory>(this->_baseUrl, "/", this->request, "");
        } else {
            const std::string rawResourcePath = value.at("parentReference").at("path");
            const auto splitPosition = rawResourcePath.find_first_of(':') + 1;
            const std::string resourcePath =
                    rawResourcePath.substr(splitPosition, rawResourcePath.size() - splitPosition) + "/";
            if (value.find("file") != value.end() && (expectedType.empty() || expectedType == "file")) {
                const std::string etag = value["eTag"];
                resource = std::make_shared<OneDriveFile>(this->_baseUrl, resourcePath + name, this->request, name,
                                                          etag);
            } else if (value.find("folder") != value.end() && (expectedType.empty() || expectedType == "folder")) {
                resource = std::make_shared<OneDriveDirectory>(this->_baseUrl, resourcePath + name, this->request,
                                                               name);
            } else {
                throw Cloud::CommunicationError("unexpected resource type");
            }
        }
        return resource;
    }

    std::string OneDriveDirectory::apiResourcePath(const std::string &path, bool children) const {
        std::string out = this->_baseUrl;
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
} // namespace CloudSync::onedrive
