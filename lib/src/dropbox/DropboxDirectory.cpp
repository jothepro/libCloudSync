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
namespace fs = std::filesystem;

namespace CloudSync::dropbox {
    std::vector<std::shared_ptr<Resource>> DropboxDirectory::list_resources() const {
        const auto resourcePath = DropboxDirectory::parsePath(this->path());
        std::vector<std::shared_ptr<Resource>> resources;
        try {
            json responseJson = this->request->POST(
                "https://api.dropboxapi.com/2/files/list_folder",
                {
                    {
                        P::HEADERS, {
                            {"Content-Type", Request::MIMETYPE_JSON}
                        }
                    }
                },
                json{
                    {"path", resourcePath},
                    {"recursive", false}
                }.dump()
            ).json();
            for (const auto &entry: responseJson.at("entries")) {
                resources.push_back(this->parseEntry(entry));
            }
            // the following code takes care of paging
            bool hasMore = responseJson.at("has_more");
            std::string cursor = responseJson.at("cursor");
            while(hasMore) {
                json responseJson = this->request->POST(
                    "https://api.dropboxapi.com/2/files/list_folder/continue",
                    {
                        {
                            P::HEADERS, {
                                {"Content-Type", Request::MIMETYPE_JSON}
                            }
                        }
                    },
                    json{
                        {"cursor", cursor}
                    }.dump()
                ).json();
                hasMore = responseJson.at("has_more");
                cursor = responseJson.at("cursor");
                for (const auto &entry: responseJson.at("entries")) {
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
                json responseJson = this->request->POST(
                    "https://api.dropboxapi.com/2/files/get_metadata",
                    {
                        {
                            P::HEADERS, {
                                {"Content-Type", Request::MIMETYPE_JSON}
                            }
                        }
                    },
                    json{
                        {"path", resourcePath}
                    }.dump()
                ).json();
                directory = std::dynamic_pointer_cast<DropboxDirectory>(this->parseEntry(responseJson, "folder"));

            } catch (...) {
                DropboxCloud::handleExceptions(std::current_exception(), resourcePath);
            }
        } else {
            directory = std::make_shared<DropboxDirectory>("/", this->request, "");
        }
        return directory;
    }

    void DropboxDirectory::remove() {
        const auto resourcePath = DropboxDirectory::parsePath(this->path());
        if (resourcePath.empty()) {
            throw PermissionDenied("deleting the root folder is not allowed");
        }
        try {
            this->request->POST(
                "https://api.dropboxapi.com/2/files/delete_v2",
                {
                    {
                        P::HEADERS, {
                            {"Content-Type", Request::MIMETYPE_JSON}
                        }
                    }
                },
                json{{"path", resourcePath}}.dump());
        } catch (...) {
            DropboxCloud::handleExceptions(std::current_exception(), resourcePath);
        }
    }

    std::shared_ptr<Directory> DropboxDirectory::create_directory(const std::string &path) const {
        const auto resourcePath = DropboxDirectory::parsePath(this->path(), path);
        std::shared_ptr<Directory> directory;
        try {
            json responseJson = this->request->POST(
                "https://api.dropboxapi.com/2/files/create_folder_v2",
                {
                    {
                        P::HEADERS, {
                            {"Content-Type", Request::MIMETYPE_JSON}
                        }
                    }
                },
                json{
                    {"path", resourcePath}
                }.dump()
            ).json();
            directory = std::dynamic_pointer_cast<DropboxDirectory>(
                            this->parseEntry(responseJson.at("metadata"), "folder"));
        } catch (...) {
            DropboxCloud::handleExceptions(std::current_exception(), resourcePath);
        }
        return directory;
    }

    std::shared_ptr<File> DropboxDirectory::create_file(const std::string &path) const {
        const auto resourcePath = DropboxDirectory::parsePath(this->path(), path);
        std::shared_ptr<File> file;
        try {
            const auto responseJson = this->request->POST(
                "https://content.dropboxapi.com/2/files/upload",
                {
                    {
                        P::HEADERS, {
                            {"Content-Type", Request::MIMETYPE_BINARY}
                        }
                    },
                    {
                        P::QUERY_PARAMS, {
                            {
                                "arg", json{
                                    {"path", resourcePath}
                                }.dump()
                            }
                        }
                    }
                },
                ""
            ).json();
            file = std::dynamic_pointer_cast<DropboxFile>(this->parseEntry(responseJson, "file"));
        } catch (...) {
            DropboxCloud::handleExceptions(std::current_exception(), resourcePath);
        }
        return file;
    }

    std::shared_ptr<File> DropboxDirectory::get_file(const std::string &path) const {
        const auto resourcePath = DropboxDirectory::parsePath(this->path(), path);
        std::shared_ptr<DropboxFile> file;
        try {
            json responseJson = this->request->POST(
                "https://api.dropboxapi.com/2/files/get_metadata",
                {
                    {
                        P::HEADERS, {
                            {"Content-Type", Request::MIMETYPE_JSON}
                        }
                    }
                },
                json{
                    {"path", resourcePath}
                }.dump()
            ).json();
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
            resource = std::make_shared<DropboxDirectory>(path, this->request, name);
        } else if (resourceType == "file") {
            resource = std::make_shared<DropboxFile>(path, this->request, name, entry.at("rev"));
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

} // namespace CloudSync::dropbox
