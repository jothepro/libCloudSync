#include "DropboxDirectory.hpp"
#include "request/Request.hpp"
#include "request/Response.hpp"
#include "DropboxFile.hpp"
#include "DropboxExceptionTranslator.hpp"
#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"
#include <filesystem>
#include <vector>

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::dropbox;
using json = nlohmann::json;
namespace fs = std::filesystem;

std::vector<std::shared_ptr<Resource>> DropboxDirectory::list_resources() const {
    std::vector<std::shared_ptr<Resource>> resources;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto path_string = m_path.generic_string();
        const json response_json = m_request->POST("https://api.dropboxapi.com/2/files/list_folder")
                ->token_auth(token)
                ->accept(Request::MIMETYPE_JSON)
                ->json_body({
                        {"path", path_string == "/" ? "" : path_string},
                        {"recursive", false}
                })->request().json();
        for (const auto &entry: response_json.at("entries")) {
            resources.push_back(this->parseEntry(entry));
        }
        // the following code takes care of paging
        bool hasMore = response_json.at("has_more");
        std::string cursor = response_json.at("cursor");
        while(hasMore) {
            const auto token = m_credentials->get_current_access_token();
            const json continue_response_json = m_request->POST(
                            "https://api.dropboxapi.com/2/files/list_folder/continue")
                    ->token_auth(token)
                    ->accept(Request::MIMETYPE_JSON)
                    ->json_body({
                            {"cursor", cursor}
                    })->request().json();
            hasMore = continue_response_json.at("has_more");
            cursor = continue_response_json.at("cursor");
            for (const auto &entry: continue_response_json.at("entries")) {
                resources.push_back(this->parseEntry(entry));
            }
        }
    } catch (...) {
        DropboxExceptionTranslator::translate(m_path);
    }
    return resources;
}

std::shared_ptr<Directory> DropboxDirectory::get_directory(const std::filesystem::path &path) const {
    const auto resource_path = append_path(path);
    std::shared_ptr<DropboxDirectory> directory;
    // get_metadata is not supported for the root folder
    if (resource_path.generic_string() == "/") {
        directory = std::make_shared<DropboxDirectory>("/", m_credentials, m_request, "");
    } else {
        try {
            const auto token = m_credentials->get_current_access_token();
            const json response_json = m_request->POST("https://api.dropboxapi.com/2/files/get_metadata")
                    ->token_auth(token)
                    ->accept(Request::MIMETYPE_JSON)
                    ->json_body({{"path", resource_path.generic_string()}})
                    ->request().json();
            directory = std::dynamic_pointer_cast<DropboxDirectory>(this->parseEntry(response_json, "folder"));

        } catch (...) {
            DropboxExceptionTranslator::translate(resource_path);
        }
    }
    return directory;
}

void DropboxDirectory::remove() {
    if (m_path.generic_string() == "/") {
        throw exceptions::resource::PermissionDenied(m_path);
    }
    try {
        const auto token = m_credentials->get_current_access_token();
        m_request->POST("https://api.dropboxapi.com/2/files/delete_v2")
                ->token_auth(token)
                ->json_body({{"path", m_path.generic_string()}})->request();
    } catch (...) {
        DropboxExceptionTranslator::translate(m_path);
    }
}

std::shared_ptr<Directory> DropboxDirectory::create_directory(const std::filesystem::path &path) const {
    const auto resource_path = append_path(path);
    std::shared_ptr<Directory> directory;
    try {
        const auto token = m_credentials->get_current_access_token();
        const json response_json = m_request->POST("https://api.dropboxapi.com/2/files/create_folder_v2")
                ->token_auth(token)
                ->accept(Request::MIMETYPE_JSON)
                ->json_body({
                        {"path", resource_path.generic_string()}
                })->request().json();
        directory = std::dynamic_pointer_cast<DropboxDirectory>(
                        this->parseEntry(response_json.at("metadata"), "folder"));
    } catch (...) {
        DropboxExceptionTranslator::translate(resource_path);
    }
    return directory;
}

std::shared_ptr<File> DropboxDirectory::create_file(const std::filesystem::path &path) const {
    const auto resource_path = append_path(path);
    std::shared_ptr<File> file;
    try {
        const auto token = m_credentials->get_current_access_token();
        const json response_json = m_request->POST("https://content.dropboxapi.com/2/files/upload")
                ->token_auth(token)
                ->content_type(Request::MIMETYPE_BINARY)
                ->accept(Request::MIMETYPE_JSON)
                ->query_param("arg", json{
                        {"path", resource_path.generic_string()}
                }.dump())->request().json();
        file = std::dynamic_pointer_cast<DropboxFile>(this->parseEntry(response_json, "file"));
    } catch (...) {
        DropboxExceptionTranslator::translate(resource_path);
    }
    return file;
}

std::shared_ptr<File> DropboxDirectory::get_file(const std::filesystem::path &path) const {
    const auto resource_path = append_path(path);
    std::shared_ptr<DropboxFile> file;
    try {
        const auto token = m_credentials->get_current_access_token();
        json response_json = m_request->POST("https://api.dropboxapi.com/2/files/get_metadata")
                ->token_auth(token)
                ->accept(Request::MIMETYPE_JSON)
                ->json_body({
                        {"path", resource_path.generic_string()}
                })->request().json();
        file = std::dynamic_pointer_cast<DropboxFile>(this->parseEntry(response_json, "file"));
    } catch (...) {
        DropboxExceptionTranslator::translate(resource_path);
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
            throw exceptions::cloud::CommunicationError("unknown resource type");
        }
    }
    if (!resourceTypeFallback.empty() && resourceType != resourceTypeFallback) {
        throw exceptions::resource::NoSuchResource(path);
    }
    if (resourceType == "folder") {
        resource = std::make_shared<DropboxDirectory>(path, m_credentials, m_request, name);
    } else if (resourceType == "file") {
        resource = std::make_shared<DropboxFile>(path, m_credentials, m_request, name, entry.at("rev"));
    }
    return resource;
}