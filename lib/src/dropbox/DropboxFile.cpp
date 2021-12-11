#include "DropboxFile.hpp"
#include "request/Request.hpp"
#include "DropboxExceptionTranslator.hpp"
#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include <nlohmann/json.hpp>

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::dropbox;
using json = nlohmann::json;

void DropboxFile::remove() {
    try {
        const auto token = m_credentials->get_current_access_token();
        m_request->POST("https://api.dropboxapi.com/2/files/delete_v2")
                ->token_auth(token)
                ->json_body({{"path", m_path.generic_string()}})->request();
    } catch (...) {
        DropboxExceptionTranslator::translate(m_path);
    }
}

bool DropboxFile::poll_change() {
    bool hasChanged = false;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->POST("https://api.dropboxapi.com/2/files/get_metadata")
                ->token_auth(token)
                ->accept(Request::MIMETYPE_JSON)
                ->json_body({{"path", m_path.generic_string()}})->request().json();
        const std::string newRevision = response_json.at("rev");
        if (this->revision() != newRevision) {
            m_revision = newRevision;
            hasChanged = true;
        }
    } catch (...) {
        DropboxExceptionTranslator::translate(m_path);
    }
    return hasChanged;
}

std::shared_ptr<request::Request> DropboxFile::prepare_read_request() const {
    const auto token = m_credentials->get_current_access_token();
    return m_request->POST("https://content.dropboxapi.com/2/files/download")
            ->token_auth(token)
            ->content_type(Request::MIMETYPE_TEXT)
            ->query_param("arg", json{{"path", m_path.generic_string()}}.dump());
}

std::string DropboxFile::read() const {
    std::string content;
    try {
        content = prepare_read_request()->request().data;
    } catch (...) {
        DropboxExceptionTranslator::translate(m_path);
    }
    return content;
}

std::vector<std::uint8_t> DropboxFile::read_binary() const {
    std::vector<std::uint8_t> content;
    try {
        content = prepare_read_request()->request_binary().data;
    } catch (...) {
        DropboxExceptionTranslator::translate(m_path);
    }
    return content;
}

std::shared_ptr<request::Request> DropboxFile::prepare_write_request() const {
    const auto token = m_credentials->get_current_access_token();
    return m_request->POST("https://content.dropboxapi.com/2/files/upload")
            ->token_auth(token)
            ->accept(Request::MIMETYPE_JSON)
            ->content_type(Request::MIMETYPE_BINARY)
            ->query_param("arg", json{
                {"path", m_path.generic_string()},
                {"mode", {
                    {".tag", "update"},
                    {"update", this->revision()}
                }}
            }.dump());
}

void DropboxFile::write(const std::string& content) {
    try {
        m_revision = prepare_write_request()->body(content)->request().json().at("rev");
    } catch(const request::exceptions::response::Conflict &e) {
        throw exceptions::resource::ResourceHasChanged(m_path);
    } catch (...) {
        DropboxExceptionTranslator::translate(m_path);
    }
}

void DropboxFile::write_binary(const std::vector<std::uint8_t> &content) {
    try {
        m_revision = prepare_write_request()->binary_body(content)->request().json().at("rev");
    } catch(const request::exceptions::response::Conflict &e) {
        throw exceptions::resource::ResourceHasChanged(m_path);
    } catch (...) {
        DropboxExceptionTranslator::translate(m_path);
    }
}
