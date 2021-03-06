#include "OAuth2CredentialsImpl.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"

using namespace CloudSync;
using namespace CloudSync::credentials;

std::string OAuth2CredentialsImpl::get_current_access_token() {
    if(m_access_token.empty() && m_refresh_token.empty()) {
        const auto response_json = m_request->POST(m_token_endpoint)
                ->postfield("client_id", m_client_id)
                ->postfield("grant_type", "authorization_code")
                ->postfield("code", m_authorization_code)
                ->postfield("redirect_uri", m_redirect_uri)
                ->postfield("code_verifier", m_code_verifier)
                ->accept(request::Request::MIMETYPE_JSON)
                ->request().json();
        m_access_token = response_json.at("access_token");
        m_refresh_token = response_json.at("refresh_token");
        m_expires = std::chrono::system_clock::now() + std::chrono::seconds(response_json.at("expires_in"));
        if(m_callback) m_callback(m_access_token, m_expires, m_refresh_token);
    } else if(!m_refresh_token.empty() && m_expires <= std::chrono::system_clock::now()) {
        auto response_json = m_request->POST(m_token_endpoint)
                ->postfield("client_id", m_client_id)
                ->postfield("grant_type", "refresh_token")
                ->postfield("refresh_token", m_refresh_token)
                ->accept(request::Request::MIMETYPE_JSON)
                ->request().json();
        m_access_token = response_json.at("access_token");
        m_expires = std::chrono::system_clock::now() + std::chrono::seconds(response_json.at("expires_in"));
        if(m_callback) m_callback(m_access_token, m_expires, m_refresh_token);
    }
    return m_access_token;
}

void OAuth2CredentialsImpl::on_token_update(
        const std::function<void(const std::string &, std::chrono::system_clock::time_point,
                                 const std::string &)> &callback) {
    m_callback = callback;
}

void OAuth2CredentialsImpl::set_request(const std::shared_ptr<request::Request>& request) {
    m_request = request;
}

void OAuth2CredentialsImpl::set_token_endpoint(const std::string &token_endpoint) {
    m_token_endpoint = token_endpoint;
}
