#pragma once

#include "OAuthCloudImpl.hpp"
#include "request/Request.hpp"
#include "request/Response.hpp"
#include "OneDriveDirectory.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace CloudSync::onedrive {
    class OneDriveCloud : public OAuthCloudImpl {
    public:
        OneDriveCloud(const std::string &drive,
                      const std::shared_ptr<credentials::OAuth2CredentialsImpl>& credentials,
                      const std::shared_ptr<request::Request> &request)
                : OAuthCloudImpl("https://graph.microsoft.com/v1.0/" + drive, "https://login.microsoftonline.com/common/oauth2/v2.0/token", credentials, request) {}

        std::shared_ptr<Directory> root() const override {
            return std::make_shared<OneDriveDirectory>(
                    m_base_url, "/",
                    m_credentials,
                    m_request, "");
        }

        std::string get_user_display_name() const override;

        void logout() override;
    };
} // namespace CloudSync::onedrive
