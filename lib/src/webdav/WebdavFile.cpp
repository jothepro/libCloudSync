#include "WebdavFile.hpp"
#include "request/Request.hpp"
#include "WebdavCloud.hpp"
#include "pugixml.hpp"

using namespace CloudSync::request;
using namespace pugi;

using P = Request::ParameterType;

namespace CloudSync::webdav {
    std::string WebdavFile::xmlQuery =
        "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
        "<d:propfind xmlns:d=\"DAV:\">\n"
        "   <d:prop>\n"
        "       <d:getetag />\n"
        "   </d:prop>\n"
        "</d:propfind>";

    void WebdavFile::rm() {
        const std::string resourcePath = this->_baseUrl + this->path();
        try {
            this->request->DELETE(resourcePath);
        } catch (...) {
            WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
        }
    }

    bool WebdavFile::pollChange(bool longPoll) {
        bool hasChanged = false;
        const std::string resourcePath = this->_baseUrl + this->path();
        try {
            if (longPoll) {
                throw Cloud::MethodNotSupportedError("Longpoll not supported");
            } else {
                const auto propfindResponse = this->request
                        ->PROPFIND(
                                resourcePath,
                                {{P::HEADERS,
                                  {{"Depth", "0"},
                                   {"Accept", Request::MIMETYPE_XML},
                                   {"Content-Type", Request::MIMETYPE_XML}}}},
                                WebdavFile::xmlQuery)
                        .xml();
                const std::string newRevision = propfindResponse
                        ->select_node("/*[local-name()='multistatus']"
                                      "/*[local-name()='response']"
                                      "/*[local-name()='propstat']"
                                      "/*[local-name()='prop']"
                                      "/*[local-name()='getetag']")
                        .node()
                        .child_value();
                if (!newRevision.empty()) {
                    if (this->revision() != newRevision) {
                        hasChanged = true;
                        this->_revision = newRevision;
                    }
                } else {
                    throw Cloud::InvalidResponse("reading XML failed: missing required 'getetag' property");
                }
            }
        } catch (...) {
            WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
        }
        return hasChanged;
    }

    std::string WebdavFile::read() const {
        std::string data;
        const std::string resourcePath = this->_baseUrl + this->path();
        try {
            data = this->request->GET(resourcePath).data;
        } catch (...) {
            WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
        }
        return data;
    }

    void WebdavFile::write(const std::string &input) {
        const std::string resourcePath = this->_baseUrl + this->path();
        try {
            const auto putResult = this->request->PUT(
                    resourcePath,
                    {{P::HEADERS, {{"If-Match", this->revision()}, {"Content-Type", Request::MIMETYPE_BINARY}}}},
                    input);
            this->_revision = putResult.headers.at("etag");
        } catch (...) {
            WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
        }
    }
} // namespace CloudSync::webdav
