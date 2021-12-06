#pragma once

#include "OAuthCloudImpl.hpp"
#include "request/Request.hpp"
#include "DropboxDirectory.hpp"

using namespace CloudSync::request;

namespace CloudSync::dropbox {
    class DropboxCloud : public OAuthCloudImpl {
    public:
        DropboxCloud(
                const std::shared_ptr<credentials::OAuth2CredentialsImpl>& credentials,
                const std::shared_ptr<request::Request> &request)
                : OAuthCloudImpl("https://www.dropbox.com", "https://api.dropbox.com/oauth2/token", credentials, request) {}

        std::shared_ptr<Directory> root() const override {
            return std::make_shared<DropboxDirectory>("/", m_credentials, m_request, "");
        }

        static void handleExceptions(const std::exception_ptr &e, const std::string &resourcePath);

        std::string get_user_display_name() const override;

        void logout() override;
    };
} // namespace CloudSync::dropbox
