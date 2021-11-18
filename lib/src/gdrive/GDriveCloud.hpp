#pragma once

#include "CloudImpl.hpp"
#include "request/Request.hpp"
#include "request/Response.hpp"
#include "GDriveDirectory.hpp"

namespace CloudSync::gdrive {
    class GDriveCloud : public CloudImpl {
    public:
        GDriveCloud(const std::string &rootName, const std::shared_ptr<request::Request> &request)
                : CloudImpl("https://www.googleapis.com/drive/v2", request), rootName(rootName) {}

        std::string getAuthorizeUrl() const override {
            return "https://accounts.google.com/o/oauth2/v2/auth";
        }

        std::string getTokenUrl() const override {
            return "https://oauth2.googleapis.com/token";
        }

        std::shared_ptr<Directory> root() const override {
            return std::make_shared<GDriveDirectory>(
                    this->baseUrl,
                    this->rootName,
                    this->rootName,
                    this->rootName,
                    "/",
                    this->request,
                    "");
        }

        static void handleExceptions(const std::exception_ptr &e, const std::string &resourcePath) {
            try {
                std::rethrow_exception(e);
            } catch (request::Response::NotFound &e) {
                try {
                    const auto errorMessage = json::parse(e.data());
                    if (errorMessage["error"]["errors"][0]["reason"] == "notFound") {
                        throw Resource::NoSuchFileOrDirectory(resourcePath);
                    } else {
                        throw Cloud::CommunicationError("unknown error response: " + e.data());
                    }
                } catch (json::exception &e) {
                    throw Cloud::InvalidResponse(e.what());
                }
            } catch (request::Response::Unauthorized &e) {
                throw Cloud::AuthorizationFailed();
            } catch (request::Response::PreconditionFailed &e) {
                throw Resource::ResourceHasChanged(resourcePath);
            } catch (request::Response::Forbidden &) {
                throw Resource::PermissionDenied(resourcePath);
            } catch (json::exception &e) {
                throw Cloud::InvalidResponse(e.what());
            } catch (request::Request::RequestException &e) {
                throw Cloud::CommunicationError(e.what());
            }
        };

        std::string getUserDisplayName() const override {
            std::string userDisplayName;
            try {
                const auto getResponse = this->request->GET("https://www.googleapis.com/userinfo/v2/me").json();
                userDisplayName = getResponse.at("name");
            } catch (...) {
                GDriveCloud::handleExceptions(std::current_exception(), "");
            }
            return userDisplayName;
        };

    private:
        std::string rootName;
    };
} // namespace CloudSync::gdrive
