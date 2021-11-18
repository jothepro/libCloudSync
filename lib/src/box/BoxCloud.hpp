#pragma once

#include "BoxDirectory.hpp"
#include "CloudImpl.hpp"
#include "request/Request.hpp"

namespace CloudSync::box {
    class BoxCloud : public CloudImpl {
    public:
        BoxCloud(const std::shared_ptr<request::Request> &request) : CloudImpl("https://api.box.com", request) {}

        std::string getAuthorizeUrl() const override {
            return "https://account.box.com/api/oauth2/authorize";
        }

        std::string getTokenUrl() const override {
            return "https://api.box.com/oauth2/token";
        }

        std::shared_ptr<Directory> root() const override {
            return std::make_shared<BoxDirectory>("0", "0", "/", this->request, "");
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
            } catch (request::Response::ResponseException &e) {
                throw Cloud::CommunicationError(e.what());
            } catch (request::Request::RequestException &e) {
                throw Cloud::CommunicationError(e.what());
            } catch (nlohmann::json::exception &e) {
                throw Cloud::InvalidResponse(e.what());
            }
        }

        std::string getUserDisplayName() const override {
            std::string userDisplayName;
            try {
                const auto getResponse = this->request->GET("https://api.box.com/2.0/users/me").json();
                userDisplayName = getResponse.at("name");
            } catch (...) {
                BoxCloud::handleExceptions(std::current_exception(), "");
            }
            return userDisplayName;
        };
    };
} // namespace CloudSync::box
