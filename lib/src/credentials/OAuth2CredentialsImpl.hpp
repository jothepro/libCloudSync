#pragma once

#include "CloudSync/OAuth2Credentials.hpp"
#include "request/Request.hpp"
#include <utility>

namespace CloudSync::credentials {
    class OAuth2CredentialsImpl : public OAuth2Credentials {
    public:
        OAuth2CredentialsImpl(
                std::string access_token,
                std::chrono::system_clock::time_point expires,
                std::string refresh_token,
                std::string client_id = "",
                std::string authorization_code = "",
                std::string redirect_uri = "",
                std::string code_verifier = "")
                : m_access_token(std::move(access_token))
                , m_expires(expires)
                , m_refresh_token(std::move(refresh_token))
                , m_client_id(std::move(client_id))
                , m_authorization_code(std::move(authorization_code))
                , m_redirect_uri(std::move(redirect_uri))
                , m_code_verifier(std::move(code_verifier)) {
        };

        [[nodiscard]] virtual std::string get_current_access_token();

        void on_token_update(const std::function<void(
                const std::string& access_token,
                std::chrono::system_clock::time_point expires,
                const std::string& refresh_token)> &callback) override;

        void set_request(const std::shared_ptr<request::Request>& request);
        void set_token_endpoint(const std::string& token_endpoint);
    private:
        std::shared_ptr<request::Request> m_request;
        std::string m_token_endpoint;
        const std::string m_client_id;
        const std::string m_authorization_code;
        const std::string m_redirect_uri;
        const std::string m_code_verifier;
        std::string m_access_token;
        std::string m_refresh_token;
        std::chrono::system_clock::time_point m_expires;
        std::function<void(
                const std::string& access_token,
                std::chrono::system_clock::time_point expires,
                const std::string& refresh_token)> m_callback;
    };
}