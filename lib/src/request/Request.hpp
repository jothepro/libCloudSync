#pragma once

#include "Response.hpp"
#include <chrono>
#include <string>
#include <unordered_map>
#include <utility>

using namespace std::literals::chrono_literals;

namespace CloudSync::request {
    class Request {
    public:
        virtual ~Request() {};

        class RequestException : public std::runtime_error {
        public:
            RequestException(const std::string &what) : std::runtime_error(what) {};
        };

        // MARK: - new interface
        enum ParameterType {
            HEADERS, QUERY_PARAMS, POSTFIELDS, MIME_POSTFIELDS, MIME_POSTFILES
        };
        enum ConfigurationOption {
            VERBOSE, FOLLOW_REDIRECT
        };

        virtual Response request(
                const std::string &verb, const std::string &url,
                const std::unordered_map<ParameterType, const std::unordered_map<std::string, std::string>> &parameters = {},
                const std::string &body = "") = 0;

        // MARK: - some aliases for known verbs
        Response
        GET(const std::string &url,
            const std::unordered_map<ParameterType, const std::unordered_map<std::string, std::string>> &parameters = {}) {
            return this->request("GET", url, parameters);
        }

        Response POST(
                const std::string &url,
                const std::unordered_map<ParameterType, const std::unordered_map<std::string, std::string>> &parameters = {},
                const std::string &body = "") {
            return this->request("POST", url, parameters, body);
        }

        Response HEAD(
                const std::string &url,
                const std::unordered_map<ParameterType, const std::unordered_map<std::string, std::string>> &parameters = {}) {
            return this->request("HEAD", url, parameters);
        }

        Response
        PUT(const std::string &url,
            const std::unordered_map<ParameterType, const std::unordered_map<std::string, std::string>> &parameters = {},
            const std::string &body = "") {
            return this->request("PUT", url, parameters, body);
        }

        Response DELETE(
                const std::string &url,
                const std::unordered_map<ParameterType, const std::unordered_map<std::string, std::string>> &parameters = {}) {
            return this->request("DELETE", url, parameters);
        }

        Response MKCOL(
                const std::string &url,
                const std::unordered_map<ParameterType, const std::unordered_map<std::string, std::string>> &parameters = {},
                const std::string &body = "") {
            return this->request("MKCOL", url, parameters, body);
        }

        Response PROPFIND(
                const std::string &url,
                const std::unordered_map<ParameterType, const std::unordered_map<std::string, std::string>> &parameters = {},
                const std::string &body = "") {
            return this->request("PROPFIND", url, parameters, body);
        }

        void setBasicAuth(const std::string &username, const std::string &password);

        /**
         * resets any kind of authentication, basicAuth or Token
         */
        void resetAuth();

        std::string getUsername();

        virtual void setTokenRequestUrl(const std::string &tokenRequestUrl);

        void virtual setProxy(
                const std::string &proxyUrl, const std::string &proxyUser = "", const std::string &proxyPassword = "");

        virtual void setOAuth2(
                const std::string &token, const std::string &refreshToken = "",
                std::chrono::system_clock::time_point expires = std::chrono::system_clock::time_point(
                        std::chrono::seconds(0)));

        std::string getCurrentRefreshToken() const;
        [[nodiscard]] virtual std::string getCurrentAccessToken() const;

        void setOption(ConfigurationOption option, bool value);

        static const std::string MIMETYPE_XML;
        static const std::string MIMETYPE_JSON;
        static const std::string MIMETYPE_BINARY;
        static const std::string MIMETYPE_TEXT;

    protected:
        /**
         * attempts to get a new OAuth2-Token
         */
        void refreshOAuth2TokenIfNeeded();

        bool optionVerbose = false;
        bool optionFollowRedirects = false;

        // OAuth2
        std::string accessToken;
        std::string refreshToken;
        std::chrono::system_clock::time_point expires;
        std::string tokenRequestUrl;
        // Basic Auth
        std::string username;
        std::string password;
        // Proxy
        std::string proxyUrl;
        std::string proxyUser;
        std::string proxyPassword;
    };
} // namespace CloudSync::request
