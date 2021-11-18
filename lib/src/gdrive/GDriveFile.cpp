#include "GDriveFile.hpp"
#include "request/Request.hpp"
#include "GDriveCloud.hpp"

using namespace CloudSync::request;
using P = Request::ParameterType;

namespace CloudSync::gdrive {
    void GDriveFile::rm() {
        try {
            this->request->DELETE(this->_baseUrl + "/files/" + this->resourceId);
        } catch (...) {
            GDriveCloud::handleExceptions(std::current_exception(), this->path());
        }
    }

    bool GDriveFile::pollChange(bool longPoll) {
        bool hasChanged = false;
        try {
            const auto responseJson =
                    this->request->GET(this->_baseUrl + "/files/" + this->resourceId,
                                       {{P::QUERY_PARAMS, {{"fields", "etag"}}}})
                            .json();
            const std::string newRevision = responseJson.at("etag");
            if (this->revision() != newRevision) {
                hasChanged = true;
                this->_revision = newRevision;
            }
        } catch (...) {
            GDriveCloud::handleExceptions(std::current_exception(), this->path());
        }
        return hasChanged;
    }

    std::string GDriveFile::read() const {
        std::string content;
        try {
            const auto responseJson =
                    this->request
                            ->GET(this->_baseUrl + "/files/" + this->resourceId,
                                  {{P::QUERY_PARAMS, {{"fields", "downloadUrl"}}}})
                            .json();
            const std::string webContentLink = responseJson.at("downloadUrl");
            content = this->request->GET(webContentLink).data;
        } catch (...) {
            GDriveCloud::handleExceptions(std::current_exception(), this->path());
        }
        return content;
    }

    void GDriveFile::write(const std::string &content) {
        try {
            const auto res =
                    this->request
                            ->PUT(
                                    "https://www.googleapis.com/upload/drive/v2/files/" + this->resourceId,
                                    {{P::QUERY_PARAMS, {{"uploadType", "media"},          {"fields",       "etag"}}},
                                     {P::HEADERS,      {{"If-Match",   this->revision()}, {"Content-Type", Request::MIMETYPE_BINARY}}}},
                                    content)
                            .json();
            this->_revision = res.at("etag");
        } catch (...) {
            GDriveCloud::handleExceptions(std::current_exception(), this->path());
        }
    }
} // namespace CloudSync::gdrive
