#include "WebdavFile.hpp"
#include "request/Request.hpp"
#include "WebdavCloud.hpp"
#include "pugixml.hpp"

using namespace CloudSync::request;
using namespace pugi;

namespace CloudSync::webdav {
    std::string WebdavFile::xmlQuery =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:propfind xmlns:d=\"DAV:\">"
            "<d:prop>"
                "<d:getetag/>"
            "</d:prop>"
        "</d:propfind>";

    void WebdavFile::remove() {
        const std::string resourcePath = m_base_url + this->path();
        try {
            this->request->DELETE(resourcePath)->send();
        } catch (...) {
            WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
        }
    }

    bool WebdavFile::poll_change(bool longPoll) {
        bool has_changed = false;
        const std::string resourcePath = m_base_url + this->path();
        try {
            if (longPoll) {
                throw Cloud::MethodNotSupportedError("Longpoll not supported");
            } else {
                const auto response_xml = this->request->PROPFIND(resourcePath)
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
            }
        } catch (...) {
            WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
        }
        return has_changed;
    }

    std::string WebdavFile::read_as_string() const {
        std::string data;
        const std::string resourcePath = m_base_url + this->path();
        try {
            data = this->request->GET(resourcePath)->send().data;
        } catch (...) {
            WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
        }
        return data;
    }

    void WebdavFile::write_string(const std::string &input) {
        const std::string resourcePath = m_base_url + this->path();
        try {
            const auto response = this->request->PUT(resourcePath)
                    ->if_match(this->revision())
                    ->content_type(Request::MIMETYPE_BINARY)
                    ->send(input);
            m_revision = response.headers.at("etag");
        } catch (...) {
            WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
        }
    }
} // namespace CloudSync::webdav
