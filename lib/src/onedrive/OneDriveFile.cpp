#include "OneDriveFile.hpp"
#include "request/Request.hpp"
#include "OneDriveExceptionTranslator.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::onedrive;

void OneDriveFile::remove() {
    try {
        const auto token = m_credentials->get_current_access_token();
        m_request->DELETE(m_resource_path)
                ->token_auth(token)
                ->request();
    } catch (...) {
        OneDriveExceptionTranslator::translate(m_path);
    }
}

bool OneDriveFile::poll_change() {
    bool has_changed = false;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->GET(m_resource_path)
                ->token_auth(token)
                ->request().json();
        const std::string new_revision = response_json.at("eTag");
        has_changed = !(new_revision == m_revision);
        m_revision = new_revision;
    } catch (...) {
        OneDriveExceptionTranslator::translate(m_path);
    }
    return has_changed;
}

std::string OneDriveFile::read() const {
    std::string data;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response = m_request->GET(m_resource_path + ":/content")
                ->token_auth(token)
                ->request();
        data = response.data;
    } catch (...) {
        OneDriveExceptionTranslator::translate(m_path);
    }
    return data;
}

std::vector<std::uint8_t> OneDriveFile::read_binary() const {
    std::vector<std::uint8_t> data;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response = m_request->GET(m_resource_path + ":/content")
                ->token_auth(token)
                ->request_binary();
        data = response.data;
    } catch (...) {
        OneDriveExceptionTranslator::translate(m_path);
    }
    return data;
}

void OneDriveFile::write(const std::string& content) {
    try {
        const auto token = m_credentials->get_current_access_token();
        const json response_json = m_request->PUT(m_resource_path + ":/content")
                ->token_auth(token)
                ->content_type(Request::MIMETYPE_BINARY)
                ->if_match(revision())
                ->body(content)->request().json();
        m_revision = response_json.at("eTag");
    } catch (...) {
        OneDriveExceptionTranslator::translate(m_path);
    }
}

void OneDriveFile::write_binary(const std::vector<std::uint8_t> &content) {
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->PUT(m_resource_path + ":/content")
                ->token_auth(token)
                ->content_type(Request::MIMETYPE_BINARY)
                ->if_match(revision())
                ->binary_body(content)->request().json();
        m_revision = response_json.at("eTag");
    } catch (...) {
        OneDriveExceptionTranslator::translate(m_path);
    }
}
