#include "BoxDirectory.hpp"
#include "BoxCloud.hpp"
#include "BoxFile.hpp"
#include "request/Request.hpp"
#include <filesystem>
#include <vector>

using namespace CloudSync::request;
using P = CloudSync::request::Request::ParameterType;
using json = nlohmann::json;
namespace fs = std::filesystem;

namespace CloudSync::box {
    std::vector<std::shared_ptr<Resource>> BoxDirectory::list_resources() const {
        std::vector<std::shared_ptr<Resource>> resources;
        try {
            json responseJson = this->request->GET(
                    "https://api.box.com/2.0/folders/" + this->resourceId + "/items").json();
            for (const auto &entry: responseJson.at("entries")) {
                resources.push_back(this->parseEntry(entry));
            }
        } catch (...) {
            BoxCloud::handleExceptions(std::current_exception(), this->path());
        }
        return resources;
    }

    std::shared_ptr<Directory> BoxDirectory::get_directory(const std::string &path) const {
        std::shared_ptr<BoxDirectory> newDir;
        // calculate "diff" between current position & wanted path. What do we need
        // to do to get there?
        const auto relativePath =
                (fs::path(this->path()) / path).lexically_normal().lexically_relative(this->path());
        try {
            if (relativePath == ".") {
                // no path change required, return current instance
                newDir = std::make_shared<BoxDirectory>(
                        this->resourceId,
                        this->parentResourceId,
                        this->path(),
                        this->request,
                        this->name());
            } else if (relativePath.begin() == --relativePath.end() && *relativePath.begin() != "..") {
                // depth of navigation = 1, get a list of all folders in folder and
                // pick the desired one.
                newDir = this->child(relativePath.string());
            } else {
                // depth of navigation > 1, slowly navigate from folder to folder
                // one by one.
                std::shared_ptr<BoxDirectory> currentDir = std::make_shared<BoxDirectory>(
                        this->resourceId,
                        this->parentResourceId,
                        this->path(),
                        this->request,
                        this->name());
                for (const auto &pathComponent: relativePath) {
                    if (pathComponent == "..") {
                        currentDir = currentDir->parent();
                    } else {
                        currentDir = std::static_pointer_cast<BoxDirectory>(
                                currentDir->get_directory(pathComponent.string()));
                    }
                }
                newDir = currentDir;
            }
        } catch (...) {
            BoxCloud::handleExceptions(std::current_exception(), path);
        }
        return newDir;
    }

    void BoxDirectory::remove() {
        try {
            if (this->path() != "/") {
                this->request->DELETE("https://api.box.com/2.0/folders/" + this->resourceId);
            } else {
                throw PermissionDenied("deleting the root folder is not allowed");
            }
        } catch (...) {
            BoxCloud::handleExceptions(std::current_exception(), this->path());
        }
    }

    std::shared_ptr<Directory> BoxDirectory::create_directory(const std::string &path) const {
        std::shared_ptr<Directory> newDir;
        std::string folderName;
        try {
            const auto baseDir = this->parent(path, folderName, true);
            json responseJson = this->request->POST(
                "https://api.box.com/2.0/folders",
                {
                    {
                        P::HEADERS, {
                            {"Content-Type", Request::MIMETYPE_JSON}
                        }
                    }
                },
                json{
                    {"name",   folderName},
                    {"parent",
                        {
                            {"id", baseDir->resourceId}
                        }
                    }
                }.dump()
            ).json();
            newDir = std::dynamic_pointer_cast<BoxDirectory>(baseDir->parseEntry(responseJson, "folder"));
        } catch (...) {
            BoxCloud::handleExceptions(std::current_exception(), path);
        }
        return newDir;
    }

    std::shared_ptr<File> BoxDirectory::create_file(const std::string &path) const {
        std::shared_ptr<File> newFile;
        std::string fileName;
        try {
            const auto baseDir = this->parent(path, fileName, true);
            const auto responseJson = this->request->POST(
                "https://upload.box.com/api/2.0/files/content",
                {
                    {
                        P::MIME_POSTFIELDS, {
                            {"attributes", json{
                                {"name",   fileName},
                                {
                                    "parent", {
                                        {"id", baseDir->resourceId}
                                    }
                                }
                            }.dump()
                            }
                        }
                    },
                    {
                        P::MIME_POSTFILES, {
                            {"file", ""}
                        }
                    }
                }
            ).json();
            newFile = std::dynamic_pointer_cast<BoxFile>(baseDir->parseEntry(responseJson.at("entries").at(0), "file"));
        } catch (...) {
            BoxCloud::handleExceptions(std::current_exception(), path);
        }
        return newFile;
    }

    std::shared_ptr<File> BoxDirectory::get_file(const std::string &path) const {
        std::shared_ptr<BoxFile> file;
        std::string fileName;
        try {
            const auto baseDir = this->parent(path, fileName);
            json responseJson =
                    this->request->GET("https://api.box.com/2.0/folders/" + baseDir->resourceId + "/items").json();
            for (const auto &entryJson: responseJson.at("entries")) {
                const auto entry = baseDir->parseEntry(entryJson);
                if (entry->name() == fileName) {
                    file = std::dynamic_pointer_cast<BoxFile>(baseDir->parseEntry(entryJson, "file"));
                    break;
                }
            }
        } catch (...) {
            BoxCloud::handleExceptions(std::current_exception(), path);
        }
        return file;
    }

    std::shared_ptr<BoxDirectory> BoxDirectory::parent() const {
        std::shared_ptr<BoxDirectory> parentDir;
        json responseJson = this->request->GET("https://api.box.com/2.0/folders/" + this->parentResourceId).json();
        return std::dynamic_pointer_cast<BoxDirectory>(
                this->parseEntry(responseJson, "folder", fs::path(this->path()).parent_path().generic_string()));
    }

    std::shared_ptr<BoxDirectory> BoxDirectory::parent(const std::string &path, std::string &folderName, bool createIfMissing) const {
        const auto relativePath =
                (fs::path(this->path()) / path).lexically_normal().lexically_relative(this->path());
        const auto relativeParentPath = relativePath.parent_path();
        folderName = relativePath.lexically_relative(relativeParentPath).generic_string();
        try {
            return std::static_pointer_cast<BoxDirectory>(this->get_directory(relativeParentPath.generic_string()));
        } catch (const NoSuchResource &e) {
            if(createIfMissing && relativeParentPath != "") {
                return std::static_pointer_cast<BoxDirectory>(this->create_directory(relativeParentPath.generic_string()));
            } else {
                throw e;
            }
        }
    }

    std::shared_ptr<BoxDirectory> BoxDirectory::child(const std::string &name) const {
        std::shared_ptr<BoxDirectory> childDir;
        json responseJson = this->request->GET("https://api.box.com/2.0/folders/" + this->resourceId + "/items").json();
        for (const auto &entryJson: responseJson.at("entries")) {
            const auto entry = this->parseEntry(entryJson);
            if (entry->name() == name) {
                childDir = std::dynamic_pointer_cast<BoxDirectory>(this->parseEntry(entryJson, "folder"));
                break;
            }
        }
        if (childDir == nullptr) {
            throw NoSuchResource((fs::path(this->path()) / name).lexically_normal().generic_string());
        }
        return childDir;
    }

    std::shared_ptr<Resource>
    BoxDirectory::parseEntry(const json &entry, const std::string &expectedType, const std::string &customPath) const {
        std::shared_ptr<Resource> resource;
        const std::string resourceId = entry.at("id");

        if (resourceId != "0") {
            const std::string etag = entry.at("etag");
            const std::string resourceType = entry.at("type");
            const std::string name = entry.at("name");

            if (!expectedType.empty() && expectedType != resourceType) {
                throw NoSuchResource("expected resource type does not match real resource type");
            }
            const auto newResourcePath = (fs::path(this->path()) / name).lexically_normal().generic_string();
            if (resourceType == "file") {
                resource = std::make_shared<BoxFile>(
                        resourceId,
                        !customPath.empty() ? customPath : newResourcePath,
                        this->request,
                        name,
                        etag);
            } else if (resourceType == "folder") {
                // extract parent resourceId from json payload if possible.
                // Otherwise fallback to pointing to current dir.
                std::string parent_resource_id;
                if (entry.contains("parent")) {
                    parent_resource_id = entry["parent"]["id"];
                } else {
                    parent_resource_id = this->resourceId;
                }
                resource = std::make_shared<BoxDirectory>(
                        resourceId,
                        parent_resource_id,
                        !customPath.empty() ? customPath : newResourcePath,
                        this->request,
                        name);
            } else {
                throw Cloud::CommunicationError("unknown resource type");
            }
        } else {
            resource = std::make_shared<BoxDirectory>("0", "0", "/", this->request, "");
        }

        return resource;
    }
} // namespace CloudSync::box
