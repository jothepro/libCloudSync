#include "DropboxFile.hpp"
#include "request/Request.hpp"
#include "DropboxCloud.hpp"
#include <nlohmann/json.hpp>

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::dropbox;
using json = nlohmann::json;

void DropboxFile::remove() {
    try {
        m_request->POST(resource_path() + "/delete_v2")
                ->send_json({{"path", this->path()}});
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), this->path());
    }
}

bool DropboxFile::poll_change() {
    bool hasChanged = false;
    try {
        const auto response_json = m_request->POST(resource_path() + "/get_metadata")
                ->accept(Request::MIMETYPE_JSON)
                ->send_json({{"path", this->path()}}).json();
        const std::string newRevision = response_json.at("rev");
        if (this->revision() != newRevision) {
            m_revision = newRevision;
            hasChanged = true;
        }
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), this->path());
    }
    return hasChanged;
}

std::string DropboxFile::read_as_string() const {
    std::string data;
    try {
        data = m_request->POST("https://content.dropboxapi.com/2/files/download")
                ->content_type(Request::MIMETYPE_TEXT)
                ->query_param("arg", json{{"path", this->path()}}.dump())
                ->send().data;
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), this->path());
    }
    return data;
}

void DropboxFile::write_string(const std::string &content) {
    try {
        const auto response_json = m_request->POST("https://content.dropboxapi.com/2/files/upload")
                ->accept(Request::MIMETYPE_JSON)
                ->content_type(Request::MIMETYPE_BINARY)
                ->query_param("arg", json{
                    {"path", this->path()},
                    {"mode", {
                        {".tag", "update"},
                        {"update", this->revision()}
                    }}
                }.dump())
                ->send(content).json();
        m_revision = response_json.at("rev");
    } catch(const request::Response::Conflict &e) {
        throw Resource::ResourceHasChanged(path());
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), this->path());
    }
}

std::string DropboxFile::resource_path() const {
    return "https://api.dropboxapi.com/2/files";
}
