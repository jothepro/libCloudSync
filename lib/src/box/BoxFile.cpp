#include "BoxFile.hpp"
#include "BoxCloud.hpp"
#include "request/Request.hpp"

using namespace CloudSync::request;

namespace CloudSync::box {
    void BoxFile::remove() {
        try {
            this->request->DELETE("https://api.box.com/2.0/files/" + this->resourceId)->send();
        } catch (...) {
            BoxCloud::handleExceptions(std::current_exception(), this->path());
        }
    }

    bool BoxFile::poll_change(bool longPoll) {
        bool hasChanged = false;
        try {
            if (longPoll) {
                // TODO implement box change longpoll
                throw std::logic_error("not yet implemented");
            } else {
                const auto responseJson = this->request->GET("https://api.box.com/2.0/files/" + this->resourceId)
                        ->accept(Request::MIMETYPE_JSON)
                        ->send().json();
                const std::string newRevision = responseJson.at("etag");
                if (this->revision() != newRevision) {
                    m_revision = newRevision;
                    hasChanged = true;
                }
            }
        } catch (...) {
            BoxCloud::handleExceptions(std::current_exception(), this->path());
        }
        return hasChanged;
    }

    std::string BoxFile::read_as_string() const {
        std::string file_content;
        try {
            file_content = this->request->GET("https://api.box.com/2.0/files/" + this->resourceId + "/content")
                    ->send().data;
        } catch (...) {
            BoxCloud::handleExceptions(std::current_exception(), this->path());
        }
        return file_content;
    }

    void BoxFile::write_string(const std::string &content) {
        try {
            const json response_json = this->request->POST("https://upload.box.com/api/2.0/files/" + this->resourceId + "/content")
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
} // namespace CloudSync::box
