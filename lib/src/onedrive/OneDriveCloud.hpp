#pragma once

#include "CloudImpl.hpp"
#include "request/Request.hpp"
#include "request/Response.hpp"
#include "OneDriveDirectory.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace CloudSync::onedrive {
    class OneDriveCloud : public CloudImpl {
    public:
        OneDriveCloud(const std::string &drive, const std::shared_ptr<request::Request> &request)
                : CloudImpl("https://graph.microsoft.com/v1.0/" + drive, request) {}

        std::string getAuthorizeUrl() const override {
            return "https://login.microsoftonline.com/common/oauth2/v2.0/authorize";
        }

        std::string getTokenUrl() const override {
            return "https://login.microsoftonline.com/common/oauth2/v2.0/token";
        }

        std::shared_ptr<Directory> root() const override {
            return std::make_shared<OneDriveDirectory>(this->baseUrl, "/", this->request, "");
        }

        static void handleExceptions(const std::exception_ptr &e, const std::string &resourcePath) {
            try {
                std::rethrow_exception(e);
            } catch (request::Response::NotFound &e) {
                throw Resource::NoSuchFileOrDirectory(resourcePath);
            } catch (request::Response::Forbidden &e) {
                throw Resource::PermissionDenied(resourcePath);
            } catch (request::Response::Unauthorized &e) {
                throw Cloud::AuthorizationFailed();
            } catch (request::Response::PreconditionFailed &e) {
                try {
                    json errorJson = json::parse(e.data());
                    const std::string errorMessage = errorJson.at("error").at("message");
                    if (errorMessage == "ETag does not match current item's value") {
                        throw Resource::ResourceHasChanged(errorMessage);
                    } else {
                        throw Cloud::CommunicationError(errorMessage);
                    }
                } catch (json::exception &e) {
                    throw Cloud::InvalidResponse(e.what());
                }
            } catch (request::Response::ResponseException &e) {
                throw Cloud::CommunicationError(e.what());
            } catch (json::exception &e) {
                throw Cloud::InvalidResponse(e.what());
            } catch (request::Request::RequestException &e) {
                throw Cloud::CommunicationError(e.what());
            }
        }

        std::string getUserDisplayName() const override {
            std::string userDisplayName;
            try {
                const auto getResponse = this->request->GET("https://graph.microsoft.com/v1.0/me").json();
                userDisplayName = getResponse.at("displayName");
            } catch (...) {
                OneDriveCloud::handleExceptions(std::current_exception(), "");
            }
            return userDisplayName;
        };
    };
} // namespace CloudSync::onedrive
