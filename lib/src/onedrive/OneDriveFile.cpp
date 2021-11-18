#include "OneDriveFile.hpp"
#include "request/Request.hpp"
#include "OneDriveCloud.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace CloudSync::request;
using P = Request::ParameterType;

namespace CloudSync::onedrive {
void OneDriveFile::rm() {
    try {
        this->request->DELETE(this->_baseUrl + ":" + this->path());
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
}

bool OneDriveFile::pollChange(bool longPoll) {
    bool hasChanged = false;
    try {
        if (longPoll) {
            throw Cloud::MethodNotSupportedError("Longpoll not supported");
        } else {
            json responseJson = this->request->GET(this->_baseUrl + ":" + this->path()).json();
            const std::string newRevision = responseJson.at("eTag");
            hasChanged = !(newRevision == this->_revision);
            this->_revision = newRevision;
        }
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
    return hasChanged;
}

std::string OneDriveFile::read() const {
    std::string fileContent;
    try {
        const auto getResult = this->request->GET(this->_baseUrl + ":" + this->path() + ":/content");
        fileContent = getResult.data;
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
    return fileContent;
}

void OneDriveFile::write(const std::string &content) {
    try {
        const auto responseJson =
            this->request
                ->PUT(
                    this->_baseUrl + ":" + this->path() + ":/content",
                    {{P::HEADERS, {{"Content-Type", Request::MIMETYPE_BINARY}, {"If-Match", this->revision()}}}},
                    content)
                .json();
        this->_revision = responseJson.at("eTag");
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), this->path());
    }
}
} // namespace CloudSync::onedrive
