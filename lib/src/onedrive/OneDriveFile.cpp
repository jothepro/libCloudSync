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
        data = prepare_read_request()->request().data;
    } catch (...) {
        OneDriveExceptionTranslator::translate(m_path);
    }
    return data;
}

std::vector<std::uint8_t> OneDriveFile::read_binary() const {
    std::vector<std::uint8_t> data;
    try {
        data = prepare_read_request()->request_binary().data;
    } catch (...) {
        OneDriveExceptionTranslator::translate(m_path);
    }
    return data;
}

std::shared_ptr<request::Request> OneDriveFile::prepare_read_request() const {
    const auto token = m_credentials->get_current_access_token();
    return m_request->GET(m_resource_path + ":/content")->token_auth(token);
}

void OneDriveFile::write(const std::string& content) {
    try {
        const json response_json = prepare_write_request()->body(content)->request().json();
        m_revision = response_json.at("eTag");
    } catch (...) {
        OneDriveExceptionTranslator::translate(m_path);
    }
}

void OneDriveFile::write_binary(const std::vector<std::uint8_t> &content) {
    try {
        const auto response_json = prepare_write_request()->binary_body(content)->request().json();
        m_revision = response_json.at("eTag");
    } catch (...) {
        OneDriveExceptionTranslator::translate(m_path);
    }
}

std::shared_ptr<request::Request> OneDriveFile::prepare_write_request() const {
    const auto token = m_credentials->get_current_access_token();
    return m_request->PUT(m_resource_path + ":/content")
            ->token_auth(token)
            ->content_type(Request::MIMETYPE_BINARY)
            ->if_match(revision());
}
