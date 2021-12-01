#include "GDriveDirectory.hpp"
#include "request/Request.hpp"
#include "GDriveCloud.hpp"
#include "GDriveFile.hpp"
#include <filesystem>

using json = nlohmann::json;
using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::gdrive;
namespace fs = std::filesystem;

std::vector<std::shared_ptr<Resource>> GDriveDirectory::list_resources() const {
    std::vector<std::shared_ptr<Resource>> resourceList;
    try {
        const auto response_json = m_request->GET(m_base_url + "/files")
                ->query_param("q", "'" + this->m_resource_id + "' in parents and trashed = false")
                ->query_param("fields", "items(kind,id,title,mimeType,etag,parents(id,isRoot))")
                ->accept(Request::MIMETYPE_JSON)
                ->send().json();
        for (const auto &file: response_json.at("items")) {
            resourceList.push_back(this->parse_file(file));
        }
    } catch (...) {
        GDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
    return resourceList;
}

std::shared_ptr<Directory> GDriveDirectory::get_directory(const std::string &path) const {
    std::shared_ptr<Directory> newDir;
    // calculate "diff" between current position & wanted path. What do we need
    // to do to get there?
    const auto relativePath = (fs::path(this->path()) / path).lexically_normal().lexically_relative(this->path());
    try {
        if (relativePath == ".") {
            // no path change required, return current dir
            newDir = std::make_shared<GDriveDirectory>(
                    m_base_url,
                    m_root_name,
                    m_resource_id,
                    m_parent_resource_id,
                    this->path(),
                    m_request,
                    name());
        } else if (relativePath.begin() == --relativePath.end() && *relativePath.begin() != "..") {
            // depth of navigation = 1, get a list of all folders in folder and
            // pick the desired one.
            newDir = this->child(relativePath.string());
        } else {
            // depth of navigation > 1, slowly navigate from folder to folder
            // one by one.
            std::shared_ptr<GDriveDirectory> currentDir = std::make_shared<GDriveDirectory>(
                    m_base_url,
                    m_root_name,
                    m_resource_id,
                    m_parent_resource_id,
                    this->path(),
                    m_request,
                    name());
            for (const auto &pathComponent: relativePath) {
                if (pathComponent == "..") {
                    currentDir = currentDir->parent();
                } else {
                    currentDir = std::static_pointer_cast<GDriveDirectory>(
                            currentDir->get_directory(pathComponent.string()));
                }
            }
            newDir = currentDir;
        }
    } catch (...) {
        GDriveCloud::handleExceptions(std::current_exception(), this->path());
    }

    return newDir;
}

void GDriveDirectory::remove() {
    try {
        if (this->path() != "/") {
            m_request->DELETE(m_base_url + "/files/" + this->m_resource_id)->send();
        } else {
            throw PermissionDenied("deleting the root folder is not allowed");
        }
    } catch (...) {
        GDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
}

std::shared_ptr<Directory> GDriveDirectory::create_directory(const std::string &path) const {
    std::shared_ptr<Directory> newDir;
    std::string folderName;
    try {
        const auto baseDir = this->parent(path, folderName, true);
        if(baseDir->child_resource_exists(folderName)) {
            throw Resource::ResourceConflict(path);
        } else {
            const auto response_json = m_request->POST(m_base_url + "/files")
                    ->accept(Request::MIMETYPE_JSON)
                    ->query_param("fields", "kind,id,title,mimeType,etag,parents(id,isRoot)")
                    ->send_json({
                            {"mimeType", "application/vnd.google-apps.folder"},
                            {"title",    folderName},
                            {
                             "parents",  {
                                             {
                                                 {"id", baseDir->m_resource_id}
                                             }
                                         }
                            }
                    }).json();
            newDir = std::dynamic_pointer_cast<Directory>(baseDir->parse_file(response_json, ResourceType::FOLDER));
        }
    } catch (...) {
        GDriveCloud::handleExceptions(std::current_exception(), path);
    }
    return newDir;
}

std::shared_ptr<File> GDriveDirectory::create_file(const std::string &path) const {
    std::string fileName;
    std::shared_ptr<File> newFile;
    try {
        const auto baseDir = this->parent(path, fileName, true);
        if(baseDir->child_resource_exists(fileName)) {
            throw Resource::ResourceConflict(path);
        } else {
            const auto response_json = m_request->POST(m_base_url + "/files")
                    ->accept(Request::MIMETYPE_JSON)
                    ->query_param("fields", "kind,id,title,mimeType,etag,parents(id,isRoot)")
                    ->send_json({
                            {"mimeType", "text/plain"},
                            {"title",    fileName},
                            {
                             "parents",  {
                                             {
                                                 {"id", baseDir->m_resource_id}
                                             }
                                         }
                            }
                    }).json();
            newFile = std::dynamic_pointer_cast<GDriveFile>(baseDir->parse_file(response_json, ResourceType::FILE));
        }
    } catch (...) {
        GDriveCloud::handleExceptions(std::current_exception(), path);
    }
    return newFile;
}

std::shared_ptr<File> GDriveDirectory::get_file(const std::string &path) const {
    std::shared_ptr<GDriveFile> file;
    std::string fileName;
    try {
        const auto baseDir = this->parent(path, fileName);
        const auto response_json = m_request->GET(m_base_url + "/files")
                ->accept(Request::MIMETYPE_JSON)
                ->query_param("q", "'" + baseDir->m_resource_id + "' in parents and title = '" + fileName + "' and trashed = false")
                ->query_param("fields", "items(kind,id,title,mimeType,etag,parents(id,isRoot))")
                ->send().json();
        if (!response_json.at("items").empty()) {
            file = std::dynamic_pointer_cast<GDriveFile>(
                    baseDir->parse_file(response_json.at("items").at(0), ResourceType::FILE));
        } else {
            throw NoSuchResource(fileName);
        }
    } catch (...) {
        GDriveCloud::handleExceptions(std::current_exception(), path);
    }
    return file;
}

std::shared_ptr<Resource>
GDriveDirectory::parse_file(const json &file, ResourceType expected_type, const std::string &custom_path) const {
    std::shared_ptr<Resource> resource;
    const std::string name = file.at("title");
    const std::string id = file.at("id");
    const std::string mime_type = file.at("mimeType");
    const std::string resource_path = (fs::path(this->path()) / name).lexically_normal().generic_string();
    std::string parent_id;
    if (file.at("parents").at(0).at("isRoot") == true) {
        parent_id = "root";
    } else {
        parent_id = file.at("parents").at(0).at("id");
    }
    if (file.at("kind") != "drive#file") {
        throw Cloud::CommunicationError("unknown file kind");
    }
    if (mime_type == "application/vnd.google-apps.folder") {
        if (expected_type != ResourceType::ANY && expected_type != ResourceType::FOLDER) {
            throw NoSuchResource(resource_path);
        }
        resource = std::make_shared<GDriveDirectory>(
                m_base_url,
                m_root_name,
                id,
                parent_id,
                !custom_path.empty() ? custom_path : resource_path,
                m_request,
                name);
    } else {
        if (expected_type != ResourceType::ANY && expected_type != ResourceType::FILE) {
            throw NoSuchResource(name);
        }
        const std::string etag = file.at("etag");
        resource = std::make_shared<GDriveFile>(
                m_base_url,
                id,
                !custom_path.empty() ? custom_path : resource_path,
                m_request,
                name,
                etag);
    }

    return resource;
}

/// @return parent of the current directory
std::shared_ptr<GDriveDirectory> GDriveDirectory::parent() const {
    const auto parentPath = fs::path(this->path()).parent_path();
    std::shared_ptr<GDriveDirectory> parentDirectory;
    if (parentPath == "/") {
        // query for root is not possible.
        parentDirectory = std::make_shared<GDriveDirectory>(
            m_base_url,
            m_root_name,
            m_root_name,
            m_root_name,
            "/",
            m_request,
            "");
    } else {
        const auto response_json = m_request->GET(m_base_url + "/files/" + this->m_parent_resource_id)
                ->accept(Request::MIMETYPE_JSON)
                ->query_param("fields", "kind,id,title,mimeType,etag,parents(id,isRoot)")
                ->send().json();
        parentDirectory = std::dynamic_pointer_cast<GDriveDirectory>(
                this->parse_file(
                        response_json,
                        ResourceType::FOLDER, parentPath.generic_string()
                )
        );
    }
    return parentDirectory;
}

/// @return parent of the given path
std::shared_ptr<GDriveDirectory> GDriveDirectory::parent(const std::string &path, std::string &folderName, bool createIfMissing) const {
    const auto relativePath = (fs::path(this->path()) / path).lexically_normal().lexically_relative(this->path());
    const auto relativeParentPath = relativePath.parent_path();
    folderName = relativePath.lexically_relative(relativeParentPath).generic_string();
    try {
        return std::static_pointer_cast<GDriveDirectory>(this->get_directory(relativeParentPath.generic_string()));
    } catch (const NoSuchResource &e) {
        if(createIfMissing && relativeParentPath != "") {
            return std::static_pointer_cast<GDriveDirectory>(this->create_directory(relativeParentPath.generic_string()));
        } else {
            throw e;
        }
    }
}

std::shared_ptr<GDriveDirectory> GDriveDirectory::child(const std::string &name) const {
    std::shared_ptr<GDriveDirectory> childDir;
    const auto response_json = m_request->GET(m_base_url + "/files")
            ->accept(Request::MIMETYPE_JSON)
            ->query_param("q", "'" + this->m_resource_id + "' in parents and title = '" + name + "' and trashed = false")
            ->query_param("fields", "items(kind,id,title,mimeType,etag,parents(id,isRoot))")
            ->send().json();
    if (!response_json.at("items").empty()) {
        childDir = std::dynamic_pointer_cast<GDriveDirectory>(
                this->parse_file(response_json.at("items").at(0), ResourceType::FOLDER));
    } else {
        throw NoSuchResource(fs::path(this->path() + "/" + name).lexically_normal().generic_string());
    }
    return childDir;
}

bool GDriveDirectory::child_resource_exists(const std::string &resource_name) const {
    const auto response_json = m_request->GET(m_base_url + "/files")
            ->accept(Request::MIMETYPE_JSON)
            ->query_param("q", "'" + this->m_resource_id + "' in parents and title = '" + resource_name + "' and trashed = false")
            ->query_param("fields", "items(kind,id,title,mimeType,etag,parents(id,isRoot))")
            ->send().json();
    return !response_json.at("items").empty();
}
