#include "Request.hpp"
#include <chrono>

using namespace std::chrono;

namespace CloudSync::request {
    const std::string Request::MIMETYPE_XML = "application/xml";
    const std::string Request::MIMETYPE_JSON = "application/json";
    const std::string Request::MIMETYPE_BINARY = "application/octet-stream";
    const std::string Request::MIMETYPE_TEXT = "text/plain";

    void Request::setOption(ConfigurationOption option, bool value) {
        switch (option) {
            case VERBOSE:
                this->optionVerbose = value;
                break;
            case FOLLOW_REDIRECT:
                this->optionFollowRedirects = value;
            default:
                break;
        }
    }

    void Request::refreshOAuth2TokenIfNeeded() {
        if (this->accessToken.empty() ||
            (!this->refreshToken.empty() && this->expires != system_clock::time_point(0s) &&
             this->expires <= system_clock::now())) {
            // token expired and needs to be refreshed

            const std::string refreshToken = this->refreshToken;
            // both refresh & access token need to be empty, otherwise the OAuth2 POST-request would attemt to authorize
            // with OAuth2
            this->refreshToken = "";
            this->accessToken = "";
            try {
                auto responseJson =
                        this->POST(
                                        this->tokenRequestUrl,
                                        {{HEADERS,    {{"Accept",     "application/json"}}},
                                         {POSTFIELDS, {{"grant_type", "refresh_token"}, {"refresh_token", refreshToken}}}})
                                .json();
                this->accessToken = responseJson.at("access_token");
                // try to also extract a refresh token, if one has been provided
                if (responseJson.find("refresh_token") != responseJson.end())
                    this->refreshToken = responseJson["refresh_token"];
                if (responseJson.find("expires_in") != responseJson.end()) {
                    this->expires = system_clock::now() + seconds(responseJson["expires_in"]);
                } else {
                    this->expires = system_clock::time_point(0s);
                }
            } catch (std::exception e) {
                throw Response::Unauthorized("error with OAuth2 Authorization: " + std::string(e.what()));
            }
        }
    }

    void Request::setBasicAuth(const std::string &username, const std::string &password) {
        this->username = username;
        this->password = password;

        // explicitly disable oauth2
        this->accessToken = "";
        this->refreshToken = "";
    }

    std::string Request::getUsername() {
        return this->username;
    }

    void Request::setOAuth2(
            const std::string &token, const std::string &refreshToken, std::chrono::system_clock::time_point expires) {
        this->accessToken = token;
        this->refreshToken = refreshToken;
        this->expires = expires;
        // explicitly disable basicAuth
        this->username = "";
        this->password = "";
    }

    void
    Request::setProxy(const std::string &proxyUrl, const std::string &proxyUser, const std::string &proxyPassword) {
        this->proxyUrl = proxyUrl;
        this->proxyUser = proxyUser;
        this->proxyPassword = proxyPassword;
    }

    void Request::setTokenRequestUrl(const std::string &tokenRequestUrl) {
        this->tokenRequestUrl = tokenRequestUrl;
    }

    std::string Request::getCurrentRefreshToken() const {
        return this->refreshToken;
    }

    std::string Request::getCurrentAccessToken() const {
        return this->accessToken;
    }

    void Request::resetAuth() {
        this->accessToken = "";
        this->refreshToken = "";
        this->username = "";
        this->password = "";
    }

} // namespace CloudSync::request
