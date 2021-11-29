#include "OneDriveFile.hpp"
#include "request/Request.hpp"
#include "OneDriveCloud.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace CloudSync::request;

namespace CloudSync::onedrive {
    void OneDriveFile::remove() {
        try {
            this->request->DELETE(m_base_url + ":" + this->path())->send();
        } catch (...) {
            OneDriveCloud::handleExceptions(std::current_exception(), this->path());
        }
    }

    bool OneDriveFile::poll_change(bool longPoll) {
        bool hasChanged = false;
        try {
            if (longPoll) {
                throw Cloud::MethodNotSupportedError("Longpoll not supported");
            } else {
                const json response_json = this->request->GET(m_base_url + ":" + this->path())->send().json();
                const std::string new_revision = response_json.at("eTag");
                hasChanged = !(new_revision == m_revision);
                m_revision = new_revision;
            }
        } catch (...) {
            OneDriveCloud::handleExceptions(std::current_exception(), this->path());
        }
        return hasChanged;
    }

    std::string OneDriveFile::read_as_string() const {
        std::string file_content;
        try {
            const auto result = this->request->GET(m_base_url + ":" + this->path() + ":/content")->send();
            file_content = result.data;
        } catch (...) {
            OneDriveCloud::handleExceptions(std::current_exception(), this->path());
        }
        return file_content;
    }

    void OneDriveFile::write_string(const std::string &content) {
        try {
            const json response_json = this->request->PUT(m_base_url + ":" + this->path() + ":/content")
                    ->content_type(Request::MIMETYPE_BINARY)
                    ->if_match(revision())
                    ->send(content).json();
            m_revision = response_json.at("eTag");
        } catch (...) {
            OneDriveCloud::handleExceptions(std::current_exception(), this->path());
        }
    }
} // namespace CloudSync::onedrive
