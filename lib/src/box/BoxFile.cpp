#include "BoxFile.hpp"
#include "BoxCloud.hpp"
#include "request/Request.hpp"

using namespace CloudSync::request;
using P = Request::ParameterType;

namespace CloudSync::box {
    void BoxFile::remove() {
        try {
            this->request->DELETE("https://api.box.com/2.0/files/" + this->resourceId);
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
                const auto responseJson = this->request->GET("https://api.box.com/2.0/files/" + this->resourceId).json();
                const std::string newRevision = responseJson.at("etag");
                if (this->revision() != newRevision) {
                    this->_revision = newRevision;
                    hasChanged = true;
                }
            }
        } catch (...) {
            BoxCloud::handleExceptions(std::current_exception(), this->path());
        }
        return hasChanged;
    }

    std::string BoxFile::read_as_string() const {
        std::string fileContent;
        try {
            fileContent = this->request->GET("https://api.box.com/2.0/files/" + this->resourceId + "/content").data;
        } catch (...) {
            BoxCloud::handleExceptions(std::current_exception(), this->path());
        }
        return fileContent;
    }

    void BoxFile::write_string(const std::string &content) {
        try {
            const auto responseJson = this->request->POST(
                "https://upload.box.com/api/2.0/files/" + this->resourceId + "/content",
                {
                    {
                        P::HEADERS, {
                            {"if-match", revision()}
                        }
                    },
                    {
                        P::MIME_POSTFIELDS, {
                             {"attributes", "{}"},
                        }
                    },
                    {
                        P::MIME_POSTFILES, {
                            {"file",       content}
                        }
                    }
                }
            ).json();
            this->_revision = responseJson.at("entries").at(0).at("etag");
        } catch (...) {
            BoxCloud::handleExceptions(std::current_exception(), this->path());
        }
    }
} // namespace CloudSync::box
