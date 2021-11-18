#include "DropboxFile.hpp"
#include "request/Request.hpp"
#include "DropboxCloud.hpp"
#include <nlohmann/json.hpp>

using namespace CloudSync::request;

using json = nlohmann::json;
using P = Request::ParameterType;

namespace CloudSync::dropbox {
void DropboxFile::rm() {
    try {
        this->request->POST(
            "https://api.dropboxapi.com/2/files/delete_v2",
            {{P::HEADERS, {{"Content-Type", Request::MIMETYPE_JSON}}}},
            json{{"path", this->path()}}.dump());
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), this->path());
    }
}

bool DropboxFile::pollChange(bool longPoll) {
    bool hasChanged = false;
    try {
        if (longPoll) {
            // TODO implement dropbox change longpoll
            throw std::logic_error("not yet implemented");
        } else {
            const auto responseJson = this->request
                                          ->POST(
                                              "https://api.dropboxapi.com/2/files/get_metadata",
                                              {{P::HEADERS, {{"Content-Type", Request::MIMETYPE_JSON}}}},
                                              json{{"path", this->path()}}.dump())
                                          .json();
            const std::string newRevision = responseJson.at("rev");
            if (this->revision() != newRevision) {
                this->_revision = newRevision;
                hasChanged = true;
            }
        }
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), this->path());
    }
    return hasChanged;
}

std::string DropboxFile::read() const {
    std::string data;
    try {
        data = this->request
                   ->POST(
                       "https://content.dropboxapi.com/2/files/download",
                       {{P::HEADERS, {{"Content-Type", Request::MIMETYPE_TEXT}}},
                        {P::QUERY_PARAMS, {{"arg", json{{"path", this->path()}}.dump()}}}},
                       "")
                   .data;
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), this->path());
    }
    return data;
}
void DropboxFile::write(const std::string &content) {
    try {
        const auto responseJson =
            this->request
                ->POST(
                    "https://content.dropboxapi.com/2/files/upload",
                    {{P::HEADERS, {{"Content-Type", Request::MIMETYPE_BINARY}}},
                     {P::QUERY_PARAMS,
                      {{"arg",
                        json{{"path", this->path()}, {"mode", {{".tag", "update"}, {"update", this->revision()}}}}
                            .dump()}}}},
                    content)
                .json();
        this->_revision = responseJson.at("rev");
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), this->path());
    }
}
} // namespace CloudSync::dropbox
