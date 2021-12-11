#include "OneDriveDirectory.hpp"
#include "request/Request.hpp"
#include "OneDriveCloud.hpp"
#include "OneDriveFile.hpp"
#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "OneDriveExceptionTranslator.hpp"
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
        const auto response_json = m_request->GET(this->api_resource_path(m_path.generic_string(), true))
                ->token_auth(m_credentials->get_current_access_token())
                ->accept(Request::MIMETYPE_JSON)
                ->request().json();

        for (const auto &value: response_json.at("value")) {
            resource_list.push_back(this->parse_drive_item(value));
        }
    } catch (...) {
        OneDriveExceptionTranslator::translate(m_path);
    }
    return resource_list;
}

std::shared_ptr<Directory> OneDriveDirectory::get_directory(const std::filesystem::path &path) const {
    std::shared_ptr<OneDriveDirectory> directory;
    const auto resource_path = append_path(path);
    if (resource_path.generic_string() == "/") {
        // if it's the root we don't need to run a query
        directory = std::make_shared<OneDriveDirectory>(m_base_url, "/", m_credentials, m_request, "");
    } else {
        try {
            const auto response_json = m_request->GET(api_resource_path(resource_path.generic_string(), false))
                    ->token_auth(m_credentials->get_current_access_token())
                    ->accept(Request::MIMETYPE_JSON)
                    ->request().json();
            directory = std::dynamic_pointer_cast<OneDriveDirectory>(parse_drive_item(response_json, "folder"));
        } catch (...) {
            OneDriveExceptionTranslator::translate(resource_path);
        }
    }
    return directory;
}

void OneDriveDirectory::remove() {
    if (m_path.generic_string() == "/") {
        throw exceptions::resource::PermissionDenied(m_path);
    } else {
        try {
            const auto token = m_credentials->get_current_access_token();
            const auto response = m_request->DELETE(m_base_url + ":" + m_path.generic_string())
                    ->token_auth(token)
                    ->request();
        } catch (...) {
            OneDriveExceptionTranslator::translate(m_path);
        }
    }
}

std::shared_ptr<Directory> OneDriveDirectory::create_directory(const std::filesystem::path &path) const {
    std::shared_ptr<OneDriveDirectory> new_directory;
    const auto resource_path = append_path(path);
    try {
        const std::string new_resource_base_path = resource_path.parent_path().generic_string();
        const std::string new_directory_name = resource_path.filename().generic_string();
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->POST(this->api_resource_path(new_resource_base_path, true))
                ->token_auth(token)
                ->accept(Request::MIMETYPE_JSON)
                ->json_body({
                        {"name", new_directory_name},
                        {"folder", json::object()},
                        {"@microsoft.graph.conflictBehavior", "fail"}
                })
                ->request().json();
        new_directory = std::dynamic_pointer_cast<OneDriveDirectory>(this->parse_drive_item(response_json, "folder"));
    } catch (...) {
        OneDriveExceptionTranslator::translate(resource_path);
    }
    return new_directory;
}

std::shared_ptr<File> OneDriveDirectory::create_file(const std::filesystem::path &path) const {
    std::shared_ptr<File> new_file;
    const auto resource_path = append_path(path);
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->PUT(m_base_url + ":" + resource_path.generic_string() + ":/content")
                ->token_auth(token)
                ->content_type(Request::MIMETYPE_BINARY)
                ->accept(Request::MIMETYPE_JSON)
                ->query_param("@microsoft.graph.conflictBehavior", "fail")
                ->body("")
                ->request().json();
        new_file = std::dynamic_pointer_cast<OneDriveFile>(this->parse_drive_item(response_json, "file"));
    } catch (...) {
        OneDriveExceptionTranslator::translate(resource_path);
    }
    return new_file;
}

std::shared_ptr<File> OneDriveDirectory::get_file(const std::filesystem::path &path) const {
    const auto resource_path = append_path(path);
    std::shared_ptr<File> file;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->GET(m_base_url + ":" + resource_path.generic_string())
                ->token_auth(token)
                ->accept(Request::MIMETYPE_JSON)
                ->request().json();
        file = std::dynamic_pointer_cast<OneDriveFile>(this->parse_drive_item(response_json, "file"));
    } catch (...) {
        OneDriveExceptionTranslator::translate(resource_path);
    }
    return file;
}

std::shared_ptr<Resource>
OneDriveDirectory::parse_drive_item(const json &value, const std::string &expectedType) const {
    std::shared_ptr<Resource> resource;
    const std::string name = value.at("name");
    // check if the returned item is the root item
    if (value.find("root") != value.end()) {
        resource = std::make_shared<OneDriveDirectory>(m_base_url, "/", m_credentials, m_request, "");
    } else {
        const std::string raw_resource_path = value.at("parentReference").at("path");
        const auto splitPosition = raw_resource_path.find_first_of(':') + 1;
        const std::string resource_path =
                raw_resource_path.substr(splitPosition, raw_resource_path.size() - splitPosition) + "/" + name;
        if (value.find("file") != value.end() && (expectedType.empty() || expectedType == "file")) {
            const std::string etag = value["eTag"];
            resource = std::make_shared<OneDriveFile>(
                    m_base_url,
                    resource_path,
                    m_credentials,
                    m_request,
                    name,
                    etag);
        } else if (value.find("folder") != value.end() && (expectedType.empty() || expectedType == "folder")) {
            resource = std::make_shared<OneDriveDirectory>(
                    m_base_url,
                    resource_path,
                    m_credentials,
                    m_request,
                    name);
        } else {
            throw exceptions::resource::NoSuchResource(resource_path);
        }
    }
    return resource;
}

std::string OneDriveDirectory::api_resource_path(const std::string &path, bool children) const {
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