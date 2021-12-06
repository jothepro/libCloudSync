#include "CloudSync/OAuth2Credentials.hpp"
#include "credentials/OAuth2CredentialsImpl.hpp"

using namespace CloudSync;

std::shared_ptr<OAuth2Credentials> OAuth2Credentials::from_authorization_code(
        const std::string &client_id,
        const std::string &authorization_code,
        const std::string &redirect_uri,
        const std::string &code_verifier) {
    return std::make_shared<credentials::OAuth2CredentialsImpl>(
            "",
            std::chrono::system_clock::time_point::min(),
            "",
            client_id,
            authorization_code,
            redirect_uri,
            code_verifier);
}

std::shared_ptr<OAuth2Credentials> OAuth2Credentials::from_access_token(
        const std::string &access_token,
        std::chrono::system_clock::time_point expires,
        const std::string& client_id,
        const std::string &refresh_token) {
    return std::make_shared<credentials::OAuth2CredentialsImpl>(
            access_token,
            expires,
            refresh_token,
            client_id);
}

std::shared_ptr<OAuth2Credentials> OAuth2Credentials::from_refresh_token(
        const std::string& client_id,
        const std::string &refresh_token) {
    return std::make_shared<credentials::OAuth2CredentialsImpl>(
        "",
        std::chrono::system_clock::time_point::min(),
        refresh_token,
        client_id);
}
