#pragma once

#include "CloudImpl.hpp"
#include "request/Request.hpp"
#include "DropboxDirectory.hpp"

using namespace CloudSync::request;
using P = Request::ParameterType;

namespace CloudSync::dropbox {
    class DropboxCloud : public CloudImpl {
    public:
        DropboxCloud(const std::shared_ptr<request::Request> &request) : CloudImpl("https://www.dropbox.com",
                                                                                   request) {}

        std::string getAuthorizeUrl() const override {
            return "https://www.dropbox.com/oauth2/authorize";
        }

        std::string getTokenUrl() const override {
            return "https://api.dropboxapi.com/oauth2/token";
        }

        std::shared_ptr<Directory> root() const override {
            return std::make_shared<DropboxDirectory>("/", this->request, "");
        }

        static void handleExceptions(const std::exception_ptr &e, const std::string &resourcePath) {
            try {
                std::rethrow_exception(e);
            } catch (request::Response::Conflict &e) {
                try {
                    json errorJson = json::parse(e.data());
                    if ((errorJson["error"][".tag"] == "path" && errorJson["error"]["path"][".tag"] == "not_found") ||
                        (errorJson["error"][".tag"] == "path_lookup" &&
                         errorJson["error"]["path_lookup"][".tag"] == "not_found")) {
                        throw Resource::NoSuchFileOrDirectory(resourcePath);
                    } else {
                        throw Cloud::CommunicationError(e.what());
                    }
                } catch (json::exception &e) {
                    throw Cloud::InvalidResponse(e.what());
                }
            } catch (request::Response::Unauthorized &e) {
                throw Cloud::AuthorizationFailed();
            } catch (request::Response::ResponseException &e) {
                throw Cloud::CommunicationError(e.what());
            } catch (Response::ParseError &e) {
                throw Cloud::InvalidResponse(e.what());
            } catch (request::Request::RequestException &e) {
                throw Cloud::CommunicationError(e.what());
            } catch (json::exception &e) {
                throw Cloud::InvalidResponse(e.what());
            }
        }

        std::string getUserDisplayName() const override {
            std::string userDisplayName;
            try {
                const auto getResponse =
                        this->request->POST("https://api.dropboxapi.com/2/users/get_current_account").json();
                userDisplayName = getResponse.at("name").at("display_name");

            } catch (...) {
                DropboxCloud::handleExceptions(std::current_exception(), "");
            }
            return userDisplayName;
        };
    };
} // namespace CloudSync::dropbox
