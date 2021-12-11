#include "GDriveFile.hpp"
#include "request/Request.hpp"
#include "GDriveExceptionTranslator.hpp"

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::gdrive;

void GDriveFile::remove() {
    try {
        const auto token = m_credentials->get_current_access_token();
        m_request->DELETE(m_resource_path)
                ->token_auth(token)
                ->request();
    } catch (...) {
        GDriveExceptionTranslator::translate(m_path);
    }
}

bool GDriveFile::poll_change() {
    bool has_changed = false;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->GET(m_resource_path)
                ->token_auth(token)
                ->query_param("fields", "etag")
                ->accept(Request::MIMETYPE_JSON)
                ->request().json();
        const std::string new_revision = response_json.at("etag");
        if (revision() != new_revision) {
            has_changed = true;
            m_revision = new_revision;
        }
    } catch (...) {
        GDriveExceptionTranslator::translate(m_path);
    }
    return has_changed;
}

std::string GDriveFile::read() const {
    std::string content;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->GET(m_resource_path)
                ->token_auth(token)
                ->query_param("fields", "downloadUrl")
                ->accept(Request::MIMETYPE_JSON)
                ->request().json();
        const std::string web_content_link = response_json.at("downloadUrl");
        content = m_request->GET(web_content_link)
                ->token_auth(token)
                ->request().data;
    } catch (...) {
        GDriveExceptionTranslator::translate(m_path);
    }
    return content;
}

std::vector<std::uint8_t> GDriveFile::read_binary() const {
    std::vector<std::uint8_t> content;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->GET(m_resource_path)
                ->token_auth(token)
                ->query_param("fields", "downloadUrl")
                ->accept(Request::MIMETYPE_JSON)
                ->request().json();
        const std::string web_content_link = response_json.at("downloadUrl");
        content = m_request->GET(web_content_link)
                ->token_auth(token)
                ->request_binary().data;
    } catch (...) {
        GDriveExceptionTranslator::translate(m_path);
    }
    return content;
}

void GDriveFile::write(const std::string& content) {
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto res = m_request->PUT("https://www.googleapis.com/upload/drive/v2/files/" + this->resourceId)
                ->token_auth(token)
                ->if_match(this->revision())
                ->content_type(Request::MIMETYPE_BINARY)
                ->accept(Request::MIMETYPE_JSON)
                ->query_param("uploadType", "media")
                ->query_param("fields", "etag")
                ->body(content)->request().json();
        m_revision = res.at("etag");
    } catch (...) {
        GDriveExceptionTranslator::translate(m_path);
    }
}

void GDriveFile::write_binary(const std::vector<std::uint8_t> &content) {
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto res = m_request->PUT("https://www.googleapis.com/upload/drive/v2/files/" + this->resourceId)
                ->token_auth(token)
                ->if_match(this->revision())
                ->content_type(Request::MIMETYPE_BINARY)
                ->accept(Request::MIMETYPE_JSON)
                ->query_param("uploadType", "media")
                ->query_param("fields", "etag")
                ->binary_body(content)->request().json();
        m_revision = res.at("etag");
    } catch (...) {
        GDriveExceptionTranslator::translate(m_path);
    }
}
