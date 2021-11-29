#include "GDriveFile.hpp"
#include "request/Request.hpp"
#include "GDriveCloud.hpp"

using namespace CloudSync::request;

namespace CloudSync::gdrive {
    void GDriveFile::remove() {
        try {
            this->request->DELETE(m_base_url + "/files/" + this->resourceId)->send();
        } catch (...) {
            GDriveCloud::handleExceptions(std::current_exception(), this->path());
        }
    }

    bool GDriveFile::poll_change(bool longPoll) {
        bool hasChanged = false;
        try {
            const auto responseJson = this->request->GET(m_base_url + "/files/" + this->resourceId)
                    ->query_param("fields", "etag")
                    ->accept(Request::MIMETYPE_JSON)
                    ->send().json();
            const std::string newRevision = responseJson.at("etag");
            if (this->revision() != newRevision) {
                hasChanged = true;
                m_revision = newRevision;
            }
        } catch (...) {
            GDriveCloud::handleExceptions(std::current_exception(), this->path());
        }
        return hasChanged;
    }

    std::string GDriveFile::read_as_string() const {
        std::string content;
        try {
            const auto responseJson = this->request->GET(m_base_url + "/files/" + this->resourceId)
                    ->query_param("fields", "downloadUrl")
                    ->accept(Request::MIMETYPE_JSON)
                    ->send().json();
            const std::string webContentLink = responseJson.at("downloadUrl");
            content = this->request->GET(webContentLink)->send().data;
        } catch (...) {
            GDriveCloud::handleExceptions(std::current_exception(), this->path());
        }
        return content;
    }

    void GDriveFile::write_string(const std::string &content) {
        try {
            const auto res = this->request->PUT("https://www.googleapis.com/upload/drive/v2/files/" + this->resourceId)
                    ->if_match(this->revision())
                    ->content_type(Request::MIMETYPE_BINARY)
                    ->accept(Request::MIMETYPE_JSON)
                    ->query_param("uploadType", "media")
                    ->query_param("fields", "etag")
                    ->send(content).json();
            m_revision = res.at("etag");
        } catch (...) {
            GDriveCloud::handleExceptions(std::current_exception(), this->path());
        }
    }
} // namespace CloudSync::gdrive
