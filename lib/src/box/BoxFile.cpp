#include "BoxFile.hpp"
#include "BoxCloud.hpp"
#include "request/Request.hpp"

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::box;

void BoxFile::remove() {
    try {
        m_request->DELETE(resource_path())->send();
    } catch (...) {
        BoxCloud::handleExceptions(std::current_exception(), this->path());
    }
}

bool BoxFile::poll_change() {
    bool hasChanged = false;
    try {
        const auto response_json = m_request->GET(resource_path())
                ->accept(Request::MIMETYPE_JSON)
                ->send().json();
        const std::string new_revision = response_json.at("etag");
        if (this->revision() != new_revision) {
            m_revision = new_revision;
            hasChanged = true;
        }
    } catch (...) {
        BoxCloud::handleExceptions(std::current_exception(), this->path());
    }
    return hasChanged;
}

std::string BoxFile::read_as_string() const {
    std::string file_content;
    try {
        file_content = m_request->GET(resource_path() + "/content")
                ->send().data;
    } catch (...) {
        BoxCloud::handleExceptions(std::current_exception(), this->path());
    }
    return file_content;
}

void BoxFile::write_string(const std::string &content) {
    try {
        const json response_json = m_request->POST("https://upload.box.com/api/2.0/files/" + this->resourceId + "/content")
                ->accept(Request::MIMETYPE_JSON)
                ->if_match(revision())
                ->mime_postfield("attributes", "{}")
                ->mime_postfile("file", content)
                ->send().json();
        m_revision = response_json.at("entries").at(0).at("etag");
    } catch (...) {
        BoxCloud::handleExceptions(std::current_exception(), this->path());
    }
}

std::string BoxFile::resource_path() const {
    return "https://api.box.com/2.0/files/" + this->resourceId;
}
