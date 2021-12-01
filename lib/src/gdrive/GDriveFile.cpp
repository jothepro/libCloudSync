#include "GDriveFile.hpp"
#include "request/Request.hpp"
#include "GDriveCloud.hpp"

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::gdrive;

void GDriveFile::remove() {
    try {
        m_request->DELETE(resource_path())->send();
    } catch (...) {
        GDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
}

bool GDriveFile::poll_change() {
    bool has_changed = false;
    try {
        const auto response_json = m_request->GET(resource_path())
                ->query_param("fields", "etag")
                ->accept(Request::MIMETYPE_JSON)
                ->send().json();
        const std::string new_revision = response_json.at("etag");
        if (revision() != new_revision) {
            has_changed = true;
            m_revision = new_revision;
        }
    } catch (...) {
        GDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
    return has_changed;
}

std::string GDriveFile::read_as_string() const {
    std::string content;
    try {
        const auto response_json = m_request->GET(resource_path())
                ->query_param("fields", "downloadUrl")
                ->accept(Request::MIMETYPE_JSON)
                ->send().json();
        const std::string web_content_link = response_json.at("downloadUrl");
        content = m_request->GET(web_content_link)->send().data;
    } catch (...) {
        GDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
    return content;
}

void GDriveFile::write_string(const std::string &content) {
    try {
        const auto res = m_request->PUT("https://www.googleapis.com/upload/drive/v2/files/" + this->resourceId)
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

std::string GDriveFile::resource_path() const {
    return m_base_url + "/files/" + this->resourceId;
}
