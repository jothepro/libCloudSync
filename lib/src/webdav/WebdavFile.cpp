#include "WebdavFile.hpp"
#include "request/Request.hpp"
#include "WebdavExceptionTranslator.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::webdav;

using namespace pugi;

const std::string WebdavFile::XML_QUERY =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<d:propfind xmlns:d=\"DAV:\">"
        "<d:prop>"
            "<d:getetag/>"
        "</d:prop>"
    "</d:propfind>";

void WebdavFile::remove() {
    try {
        m_request->DELETE(m_resource_path)
            ->basic_auth(m_credentials->username(), m_credentials->password())
            ->request();
    } catch (...) {
        WebdavExceptionTranslator::translate(m_path);
    }
}

bool WebdavFile::poll_change() {
    bool has_changed = false;
    try {

        const auto response = m_request->PROPFIND(m_resource_path)
                ->basic_auth(m_credentials->username(), m_credentials->password())
                ->header("Depth", "0")
                ->accept(Request::MIMETYPE_XML)
                ->content_type(Request::MIMETYPE_XML)
                ->body(XML_QUERY)
                ->request();

        const std::string new_revision = response.xml()
                ->select_node("/*[local-name()='multistatus']"
                              "/*[local-name()='response']"
                              "/*[local-name()='propstat']"
                              "/*[local-name()='prop']"
                              "/*[local-name()='getetag']")
                .node()
                .child_value();
        if (!new_revision.empty()) {
            if (revision() != new_revision) {
                has_changed = true;
                m_revision = new_revision;
            }
        } else {
            throw exceptions::cloud::InvalidResponse("reading XML failed: missing required 'getetag' property");
        }
    } catch (...) {
        WebdavExceptionTranslator::translate(m_path);
    }
    return has_changed;
}

std::string WebdavFile::read() const {
    std::string result;
    try {
        result = m_request->GET(m_resource_path)
                ->basic_auth(m_credentials->username(), m_credentials->password())
                ->request().data;
    } catch (...) {
        WebdavExceptionTranslator::translate(m_path);
    }
    return result;
}

std::vector<std::uint8_t> WebdavFile::read_binary() const {
    std::vector<std::uint8_t> result;
    try {
        result = m_request->GET(m_resource_path)
                        ->basic_auth(m_credentials->username(), m_credentials->password())
                        ->request_binary().data;
    } catch (...) {
        WebdavExceptionTranslator::translate(m_path);
    }
    return result;
}

void WebdavFile::write(const std::string& content) {
    try {
        const auto response = m_request->PUT(m_resource_path)
                ->basic_auth(m_credentials->username(), m_credentials->password())
                ->if_match(revision())
                ->content_type(Request::MIMETYPE_BINARY)
                ->body(content)
                ->request();
        m_revision = response.headers.at("etag");
    } catch (...) {
        WebdavExceptionTranslator::translate(m_path);
    }
}

void WebdavFile::write_binary(const std::vector<std::uint8_t> &content) {
    try {
        const auto response = m_request->PUT(m_resource_path)
                ->basic_auth(m_credentials->username(), m_credentials->password())
                ->if_match(revision())
                ->content_type(Request::MIMETYPE_BINARY)
                ->binary_body(content)
                ->request();
        m_revision = response.headers.at("etag");
    } catch (...) {
        WebdavExceptionTranslator::translate(m_path);
    }
}
