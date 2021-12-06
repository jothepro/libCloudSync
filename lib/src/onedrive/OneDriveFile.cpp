#include "OneDriveFile.hpp"
#include "request/Request.hpp"
#include "OneDriveCloud.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::onedrive;

void OneDriveFile::remove() {
    try {
        const auto token = m_credentials->get_current_access_token();
        m_request->DELETE(resource_path())
                ->token_auth(token)
                ->send();
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
}

bool OneDriveFile::poll_change() {
    bool has_changed = false;
    try {
        const auto token = m_credentials->get_current_access_token();
        const json response_json = m_request->GET(resource_path())
                ->token_auth(token)
                ->send().json();
        const std::string new_revision = response_json.at("eTag");
        has_changed = !(new_revision == m_revision);
        m_revision = new_revision;
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
    return has_changed;
}

std::string OneDriveFile::read_as_string() const {
    std::string file_content;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto result = m_request->GET(resource_path() + ":/content")
                ->token_auth(token)
                ->send();
        file_content = result.data;
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
    return file_content;
}

void OneDriveFile::write_string(const std::string &content) {
    try {
        const auto token = m_credentials->get_current_access_token();
        const json response_json = m_request->PUT(resource_path() + ":/content")
                ->token_auth(token)
                ->content_type(Request::MIMETYPE_BINARY)
                ->if_match(revision())
                ->send(content).json();
        m_revision = response_json.at("eTag");
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
}

std::string OneDriveFile::resource_path() const {
    return m_base_url + ":" + path();
}
