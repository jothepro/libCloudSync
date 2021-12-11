#include "GDriveDirectory.hpp"
#include "request/Request.hpp"
#include "GDriveExceptionTranslator.hpp"
#include "GDriveFile.hpp"
#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"
#include <filesystem>

using json = nlohmann::json;
using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::gdrive;
namespace fs = std::filesystem;

std::vector<std::shared_ptr<Resource>> GDriveDirectory::list_resources() const {
    std::vector<std::shared_ptr<Resource>> resource_list;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->GET(m_base_url + "/files")
                ->token_auth(token)
                ->query_param("q", "'" + this->m_resource_id + "' in parents and trashed = false")
                ->query_param("fields", "items(kind,id,title,mimeType,etag,parents(id,isRoot))")
                ->accept(Request::MIMETYPE_JSON)
                ->request().json();
        for (const auto &file: response_json.at("items")) {
            resource_list.push_back(this->parse_file(file));
        }
    } catch (...) {
        GDriveExceptionTranslator::translate(path());
    }
    return resource_list;
}

std::shared_ptr<Directory> GDriveDirectory::get_directory(const std::filesystem::path &path) const {
    std::shared_ptr<Directory> newDir;
    // calculate "diff" between current position & wanted path. What do we need
    // to do to get there?
    const auto relative_path = append_path(path).lexically_relative(m_path);
    try {
        if (relative_path == ".") {
            // no path change required, return current dir
            newDir = std::make_shared<GDriveDirectory>(
                    m_base_url,
                    m_root_name,
                    m_resource_id,
                    m_parent_resource_id,
                    m_path,
                    m_credentials,
                    m_request,
                    m_name);
        } else if (relative_path.begin() == --relative_path.end() && *relative_path.begin() != "..") {
            // depth of navigation = 1, get a list of all folders in folder and
            // pick the desired one.
            newDir = this->child(relative_path.string());
        } else {
            // depth of navigation > 1, slowly navigate from folder to folder
            // one by one.
            std::shared_ptr<GDriveDirectory> current_dir = std::make_shared<GDriveDirectory>(
                    m_base_url,
                    m_root_name,
                    m_resource_id,
                    m_parent_resource_id,
                    m_path,
                    m_credentials,
                    m_request,
                    name());
            for (const auto &path_component: relative_path) {
                if (path_component == "..") {
                    current_dir = current_dir->parent();
                } else {
                    current_dir = std::static_pointer_cast<GDriveDirectory>(
                            current_dir->get_directory(path_component));
                }
            }
            newDir = current_dir;
        }
    } catch (...) {
        GDriveExceptionTranslator::translate(m_path);
    }
    return newDir;
}

void GDriveDirectory::remove() {
    try {
        if (m_path.generic_string() == "/") {
            throw exceptions::resource::PermissionDenied(m_path);
        } else {
            const auto token = m_credentials->get_current_access_token();
            m_request->DELETE(m_base_url + "/files/" + this->m_resource_id)
                    ->token_auth(token)
                    ->request();
        }
    } catch (...) {
        GDriveExceptionTranslator::translate(path());
    }
}

std::shared_ptr<Directory> GDriveDirectory::create_directory(const std::filesystem::path &path) const {
    std::shared_ptr<Directory> new_directory;
    std::string folder_name;
    const auto full_path = append_path(path);
    const auto base_directory = this->parent(path.generic_string(), folder_name, true);
    try {
        if(base_directory->child_resource_exists(folder_name)) {
            throw exceptions::resource::ResourceConflict(full_path);
        } else {
            const auto token = m_credentials->get_current_access_token();
            const auto response_json = m_request->POST(m_base_url + "/files")
                    ->token_auth(token)
                    ->accept(Request::MIMETYPE_JSON)
                    ->query_param("fields", "kind,id,title,mimeType,etag,parents(id,isRoot)")
                    ->json_body({
                            {"mimeType", "application/vnd.google-apps.folder"},
                            {"title",    folder_name},
                            {
                             "parents",  {
                                             {
                                                 {"id", base_directory->m_resource_id}
                                             }
                                         }
                            }
                    })->request().json();
            new_directory = std::dynamic_pointer_cast<Directory>(base_directory->parse_file(response_json, ResourceType::FOLDER));
        }
    } catch (...) {
        GDriveExceptionTranslator::translate(full_path);
    }
    return new_directory;
}

std::shared_ptr<File> GDriveDirectory::create_file(const std::filesystem::path &path) const {
    std::string file_name;
    std::shared_ptr<File> new_file;
    const auto base_dir = this->parent(path.generic_string(), file_name, true);
    try {
        if(base_dir->child_resource_exists(file_name)) {
            throw exceptions::resource::ResourceConflict(path);
        } else {
            const auto token = m_credentials->get_current_access_token();
            const auto response_json = m_request->POST(m_base_url + "/files")
                    ->token_auth(token)
                    ->accept(Request::MIMETYPE_JSON)
                    ->query_param("fields", "kind,id,title,mimeType,etag,parents(id,isRoot)")
                    ->json_body({
                            {"mimeType", "text/plain"},
                            {"title",    file_name},
                            {
                             "parents",  {
                                             {
                                                 {"id", base_dir->m_resource_id}
                                             }
                                         }
                            }
                    })->request().json();
            new_file = std::dynamic_pointer_cast<GDriveFile>(base_dir->parse_file(response_json, ResourceType::FILE));
        }
    } catch (...) {
        GDriveExceptionTranslator::translate(base_dir->path() / path);
    }
    return new_file;
}

std::shared_ptr<File> GDriveDirectory::get_file(const std::filesystem::path &path) const {
    std::shared_ptr<GDriveFile> file;
    std::string file_name;
    const auto full_path = append_path(path);
    try {
        const auto base_dir = this->parent(path.generic_string(), file_name);
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->GET(m_base_url + "/files")
                ->token_auth(token)
                ->accept(Request::MIMETYPE_JSON)
                ->query_param("q", "'" + base_dir->m_resource_id + "' in parents and title = '" + file_name + "' and trashed = false")
                ->query_param("fields", "items(kind,id,title,mimeType,etag,parents(id,isRoot))")
                ->request().json();
        if (!response_json.at("items").empty()) {
            file = std::dynamic_pointer_cast<GDriveFile>(
                    base_dir->parse_file(response_json.at("items").at(0), ResourceType::FILE));
        } else {
            throw exceptions::resource::NoSuchResource(full_path);
        }
    } catch (...) {
        GDriveExceptionTranslator::translate(full_path);
    }
    return file;
}

std::shared_ptr<Resource>
GDriveDirectory::parse_file(const json &file, ResourceType expected_type, const std::string &custom_path) const {
    std::shared_ptr<Resource> resource;
    const std::string name = file.at("title");
    const std::string id = file.at("id");
    const std::string mime_type = file.at("mimeType");
    const std::string resource_path = (m_path / name).lexically_normal().generic_string();
    std::string parent_id;
    if (file.at("parents").at(0).at("isRoot") == true) {
        parent_id = "root";
    } else {
        parent_id = file.at("parents").at(0).at("id");
    }
    if (file.at("kind") != "drive#file") {
        throw exceptions::cloud::CommunicationError("unknown file kind");
    }
    if (mime_type == "application/vnd.google-apps.folder") {
        if (expected_type != ResourceType::ANY && expected_type != ResourceType::FOLDER) {
            throw exceptions::resource::NoSuchResource(resource_path);
        }
        resource = std::make_shared<GDriveDirectory>(
                m_base_url,
                m_root_name,
                id,
                parent_id,
                !custom_path.empty() ? custom_path : resource_path,
                m_credentials,
                m_request,
                name);
    } else {
        if (expected_type != ResourceType::ANY && expected_type != ResourceType::FILE) {
            throw exceptions::resource::NoSuchResource(name);
        }
        const std::string etag = file.at("etag");
        resource = std::make_shared<GDriveFile>(
                m_base_url,
                id,
                !custom_path.empty() ? custom_path : resource_path,
                m_credentials,
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
            m_credentials,
            m_request,
            "");
    } else {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->GET(m_base_url + "/files/" + this->m_parent_resource_id)
                ->token_auth(token)
                ->accept(Request::MIMETYPE_JSON)
                ->query_param("fields", "kind,id,title,mimeType,etag,parents(id,isRoot)")
                ->request().json();
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
        return std::static_pointer_cast<GDriveDirectory>(get_directory(relativeParentPath.generic_string()));
    } catch (const exceptions::resource::NoSuchResource &e) {
        if(createIfMissing && relativeParentPath != "") {
            return std::static_pointer_cast<GDriveDirectory>(this->create_directory(relativeParentPath.generic_string()));
        } else {
            throw e;
        }
    }
}

std::shared_ptr<GDriveDirectory> GDriveDirectory::child(const std::string &name) const {
    std::shared_ptr<GDriveDirectory> childDir;
    const auto token = m_credentials->get_current_access_token();
    const auto response_json = m_request->GET(m_base_url + "/files")
            ->token_auth(token)
            ->accept(Request::MIMETYPE_JSON)
            ->query_param("q", "'" + this->m_resource_id + "' in parents and title = '" + name + "' and trashed = false")
            ->query_param("fields", "items(kind,id,title,mimeType,etag,parents(id,isRoot))")
            ->request().json();
    if (!response_json.at("items").empty()) {
        childDir = std::dynamic_pointer_cast<GDriveDirectory>(
                this->parse_file(response_json.at("items").at(0), ResourceType::FOLDER));
    } else {
        throw exceptions::resource::NoSuchResource(append_path(name));
    }
    return childDir;
}

bool GDriveDirectory::child_resource_exists(const std::string &resource_name) const {
    const auto token = m_credentials->get_current_access_token();
    const auto response_json = m_request->GET(m_base_url + "/files")
            ->token_auth(token)
            ->accept(Request::MIMETYPE_JSON)
            ->query_param("q", "'" + this->m_resource_id + "' in parents and title = '" + resource_name + "' and trashed = false")
            ->query_param("fields", "items(kind,id,title,mimeType,etag,parents(id,isRoot))")
            ->request().json();
    return !response_json.at("items").empty();
}
