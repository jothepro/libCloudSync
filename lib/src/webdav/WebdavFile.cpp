#include "WebdavFile.hpp"
#include "request/Request.hpp"
#include "WebdavCloud.hpp"
#include <pugixml.hpp>

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::webdav;
using namespace pugi;

std::string WebdavFile::xmlQuery =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<d:propfind xmlns:d=\"DAV:\">"
        "<d:prop>"
            "<d:getetag/>"
        "</d:prop>"
    "</d:propfind>";

void WebdavFile::remove() {
    try {
        m_request->DELETE(resource_path())->send();
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), resource_path());
    }
}

bool WebdavFile::poll_change() {
    bool has_changed = false;
    try {
        const auto response_xml = m_request->PROPFIND(resource_path())
                ->header("Depth", "0")
                ->accept(Request::MIMETYPE_XML)
                ->content_type(Request::MIMETYPE_XML)
                ->send(xmlQuery).xml();
        const std::string new_revision = response_xml
                ->select_node("/*[local-name()='multistatus']"
                              "/*[local-name()='response']"
                              "/*[local-name()='propstat']"
                              "/*[local-name()='prop']"
                              "/*[local-name()='getetag']")
                .node()
                .child_value();
        if (!new_revision.empty()) {
            if (this->revision() != new_revision) {
                has_changed = true;
                m_revision = new_revision;
            }
        } else {
            throw Cloud::InvalidResponse("reading XML failed: missing required 'getetag' property");
        }
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), resource_path());
    }
    return has_changed;
}

std::string WebdavFile::read_as_string() const {
    std::string data;
    try {
        data = m_request->GET(resource_path())->send().data;
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), resource_path());
    }
    return data;
}

void WebdavFile::write_string(const std::string &input) {
    try {
        const auto response = m_request->PUT(resource_path())
                ->if_match(this->revision())
                ->content_type(Request::MIMETYPE_BINARY)
                ->send(input);
        m_revision = response.headers.at("etag");
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), resource_path());
    }
}

std::string WebdavFile::resource_path() const {
    return m_base_url + path();
}
