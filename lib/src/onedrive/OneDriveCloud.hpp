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

        static void handleExceptions(const std::exception_ptr &e, const std::string &resourcePath);

        std::string getUserDisplayName() const override;

        void logout() override;
    };
} // namespace CloudSync::onedrive
