#pragma once

#include <utility>

#include "OAuthCloudImpl.hpp"
#include "request/Request.hpp"
#include "request/Response.hpp"
#include "GDriveDirectory.hpp"

namespace CloudSync::gdrive {
    class GDriveCloud : public OAuthCloudImpl {
    public:
        GDriveCloud(std::string root_name,
                    const std::shared_ptr<credentials::OAuth2CredentialsImpl> credentials,
                    const std::shared_ptr<request::Request> &request)
                : OAuthCloudImpl("https://www.googleapis.com/drive/v2", "https://oauth2.googleapis.com/token", credentials, request), m_root_name(std::move(root_name)) {}

        std::shared_ptr<Directory> root() const override {
            return std::make_shared<GDriveDirectory>(
                    m_base_url,
                    m_root_name,
                    m_root_name,
                    m_root_name,
                    "/",
                    m_credentials,
                    m_request,
                    "");
        }

        std::string get_user_display_name() const override;

        void logout() override;

    private:
        std::string m_root_name;
    };
}
