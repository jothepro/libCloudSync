#include "DropboxFile.hpp"
#include "request/Request.hpp"
#include "DropboxCloud.hpp"
#include <nlohmann/json.hpp>

using namespace CloudSync::request;

using json = nlohmann::json;

namespace CloudSync::dropbox {
    void DropboxFile::remove() {
        try {
            this->request->POST("https://api.dropboxapi.com/2/files/delete_v2")
                    ->send_json({{"path", this->path()}});
        } catch (...) {
            DropboxCloud::handleExceptions(std::current_exception(), this->path());
        }
    }

    bool DropboxFile::poll_change(bool longPoll) {
        bool hasChanged = false;
        try {
            if (longPoll) {
                // TODO implement dropbox change longpoll
                throw std::logic_error("not yet implemented");
            } else {
                const auto responseJson = this->request->POST("https://api.dropboxapi.com/2/files/get_metadata")
                        ->accept(Request::MIMETYPE_JSON)
                        ->send_json({{"path", this->path()}}).json();
                const std::string newRevision = responseJson.at("rev");
                if (this->revision() != newRevision) {
                    m_revision = newRevision;
                    hasChanged = true;
                }
            }
        } catch (...) {
            DropboxCloud::handleExceptions(std::current_exception(), this->path());
        }
        return hasChanged;
    }

    std::string DropboxFile::read_as_string() const {
        std::string data;
        try {
            data = this->request->POST("https://content.dropboxapi.com/2/files/download")
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
            const auto responseJson = this->request->POST("https://content.dropboxapi.com/2/files/upload")
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
            m_revision = responseJson.at("rev");
        } catch(const request::Response::Conflict &e) {
            throw Resource::ResourceHasChanged(path());
        } catch (...) {
            DropboxCloud::handleExceptions(std::current_exception(), this->path());
        }
    }
} // namespace CloudSync::dropbox
