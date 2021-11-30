#include "Request.hpp"
#include <chrono>

using namespace std::chrono;

namespace CloudSync::request {

    const std::string Request::MIMETYPE_XML = "application/xml";
    const std::string Request::MIMETYPE_JSON = "application/json";
    const std::string Request::MIMETYPE_BINARY = "application/octet-stream";
    const std::string Request::MIMETYPE_TEXT = "text/plain";

    void Request::set_follow_redirects(bool follow) {
        this->m_option_follow_redirects = follow;
    }

    void Request::set_verbose(bool verbose) {
        this->m_option_verbose = verbose;
    }

    void Request::refresh_oauth2_token_if_needed() {
        if (m_access_token.empty() ||
            (!m_refresh_token.empty() && this->m_expires != system_clock::time_point(0s) &&
             this->m_expires <= system_clock::now())) {
            // token expired and needs to be refreshed

            const std::string refreshToken = m_refresh_token;
            // both refresh & access token need to be empty, otherwise the OAuth2 POST-request would attemt to authorize
            // with OAuth2
            m_refresh_token.clear();
            m_access_token.clear();
            try {
                auto response_json = this->POST(this->m_token_request_url)
                        ->accept(Request::MIMETYPE_JSON)
                        ->postfield("grant_type", "refresh_token")
                        ->postfield("refresh_token", refreshToken)
                        ->send().json();
                m_access_token = response_json.at("access_token");
                // try to also extract a refresh token, if one has been provided
                if (response_json.find("refresh_token") != response_json.end())
                    m_refresh_token = response_json["refresh_token"];
                if (response_json.find("expires_in") != response_json.end()) {
                    this->m_expires = system_clock::now() + seconds(response_json["expires_in"]);
                } else {
                    this->m_expires = system_clock::time_point(0s);
                }
            } catch (const std::exception& e) {
                throw Response::Unauthorized("error with OAuth2 Authorization: " + std::string(e.what()));
            }
        }
    }

    void Request::set_basic_auth(const std::string &username, const std::string &password) {
        m_username = username;
        m_password = password;

        // explicitly disable oauth2
        m_access_token.clear();
        m_refresh_token.clear();
    }

    std::string Request::get_username() {
        return m_username;
    }

    void Request::set_oauth2(
            const std::string &token, const std::string &refreshToken, std::chrono::system_clock::time_point expires) {
        m_access_token = token;
        m_refresh_token = refreshToken;
        m_expires = expires;
        // explicitly disable basicAuth
        m_username.clear();
        m_password.clear();
    }

    void Request::set_proxy(const std::string &proxyUrl, const std::string &proxyUser, const std::string &proxyPassword) {
        m_proxy_url = proxyUrl;
        m_proxy_user = proxyUser;
        m_proxy_password = proxyPassword;
    }

    void Request::set_token_request_url(const std::string &tokenRequestUrl) {
        m_token_request_url = tokenRequestUrl;
    }

    std::string Request::get_current_refresh_token() const {
        return m_refresh_token;
    }

    std::string Request::get_current_access_token() const {
        return m_access_token;
    }

    void Request::reset_auth() {
        m_access_token.clear();
        m_refresh_token.clear();
        m_username.clear();
        m_password.clear();
    }

    Response Request::send_json(const nlohmann::json &json_data) {
        content_type(Request::MIMETYPE_JSON);
        return send(json_data.dump());
    }

    std::shared_ptr<Request> Request::accept(const std::string &mimetype) {
        return header("Accept", mimetype);
    }

    std::shared_ptr<Request> Request::content_type(const std::string &mimetype) {
        return header("Content-Type", mimetype);
    }

    std::shared_ptr<Request> Request::if_match(const std::string &etag) {
        return header("If-Match", etag);
    }

} // namespace CloudSync::request
