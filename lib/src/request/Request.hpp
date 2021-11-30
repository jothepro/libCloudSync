#pragma once

#include "Response.hpp"
#include <chrono>
#include <string>
#include <unordered_map>
#include <utility>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#undef DELETE

using namespace std::literals::chrono_literals;

namespace CloudSync::request {
    class Request {
    public:
        virtual ~Request() = default;

        class RequestException : public std::runtime_error {
        public:
            RequestException(const std::string &what) : std::runtime_error(what) {};
        };

        virtual std::shared_ptr<Request> request(const std::string &verb, const std::string &url) = 0;

        // MARK: - some aliases for known verbs
        std::shared_ptr<Request> GET(const std::string &url) {
            return this->request("GET", url);
        }

        std::shared_ptr<Request> POST(const std::string &url) {
            return this->request("POST", url);
        }

        std::shared_ptr<Request> HEAD(const std::string &url) {
            return this->request("HEAD", url);
        }

        std::shared_ptr<Request> PUT(const std::string &url) {
            return this->request("PUT", url);
        }

        std::shared_ptr<Request> DELETE(const std::string &url) {
            return this->request("DELETE", url);
        }

        std::shared_ptr<Request> MKCOL(const std::string &url) {
            return this->request("MKCOL", url);
        }

        std::shared_ptr<Request> PROPFIND(const std::string &url) {
            return this->request("PROPFIND", url);
        }

        virtual std::shared_ptr<Request> header(const std::string& key, const std::string& value) = 0;
        std::shared_ptr<Request> accept(const std::string& mimetype);
        std::shared_ptr<Request> content_type(const std::string& mimetype);
        std::shared_ptr<Request> if_match(const std::string& etag);
        virtual std::shared_ptr<Request> query_param(const std::string& key, const std::string& value) = 0;
        virtual std::shared_ptr<Request> postfield(const std::string& key, const std::string& value) = 0;
        virtual std::shared_ptr<Request> mime_postfield(const std::string& key, const std::string& value) = 0;
        virtual std::shared_ptr<Request> mime_postfile(const std::string& key, const std::string& value) = 0;

        virtual Response send(const std::optional<std::string>& body = std::nullopt) = 0;
        Response send_json(const nlohmann::json& json_data);


        void set_basic_auth(const std::string &username, const std::string &password);

        /**
         * resets any kind of authentication, basicAuth or Token
         */
        virtual void reset_auth();

        std::string get_username();

        virtual void set_token_request_url(const std::string &tokenRequestUrl);

        virtual void set_proxy(
                const std::string &proxyUrl, const std::string &proxyUser = "", const std::string &proxyPassword = "");

        virtual void set_oauth2(
                const std::string &token, const std::string &refreshToken = "",
                std::chrono::system_clock::time_point expires = std::chrono::system_clock::time_point(
                        std::chrono::seconds(0)));

        std::string get_current_refresh_token() const;
        [[nodiscard]] virtual std::string get_current_access_token() const;

        void set_follow_redirects(bool follow);
        void set_verbose(bool verbose);

        static const std::string MIMETYPE_XML;
        static const std::string MIMETYPE_JSON;
        static const std::string MIMETYPE_BINARY;
        static const std::string MIMETYPE_TEXT;
    protected:
        /**
         * attempts to get a new OAuth2-Token
         */
        void refresh_oauth2_token_if_needed();

        bool m_option_verbose = false;
        bool m_option_follow_redirects = false;

        // OAuth2
        std::string m_access_token;
        std::string m_refresh_token;
        std::chrono::system_clock::time_point m_expires;
        std::string m_token_request_url;
        // Basic Auth
        std::string m_username;
        std::string m_password;
        // Proxy
        std::string m_proxy_url;
        std::string m_proxy_user;
        std::string m_proxy_password;
    };
} // namespace CloudSync::request
