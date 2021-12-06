#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <functional>

namespace CloudSync {
    class OAuth2Credentials {
    public:
        virtual ~OAuth2Credentials() = default;
        [[nodiscard]] static std::shared_ptr<OAuth2Credentials> from_authorization_code(
                const std::string& client_id,
                const std::string& authorization_code,
                const std::string& redirect_uri,
                const std::string& code_verifier);
        [[nodiscard]] static std::shared_ptr<OAuth2Credentials> from_access_token(
                const std::string& access_token,
                std::chrono::system_clock::time_point expires = std::chrono::system_clock::time_point::max(),
                const std::string& client_id = "",
                const std::string& refresh_token = "");
        [[nodiscard]] static std::shared_ptr<OAuth2Credentials> from_refresh_token(
                const std::string& client_id,
                const std::string& refresh_token);

        virtual void on_token_update(const std::function<void(
                        const std::string& access_token,
                        std::chrono::system_clock::time_point expires,
                        const std::string& refresh_token)> &callback) = 0;
    };
}