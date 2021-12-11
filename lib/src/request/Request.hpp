#pragma once

#include "BinaryResponse.hpp"
#include "StringResponse.hpp"
#include "CloudSync/OAuth2Credentials.hpp"
#include <chrono>
#include <string>
#include <unordered_map>
#include <utility>
#include <optional>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#undef DELETE

using namespace std::literals::chrono_literals;

namespace CloudSync::request {
    class Request {
    public:
        virtual ~Request() = default;

        virtual std::shared_ptr<Request> resource(const std::string &verb, const std::string &url) = 0;

        // MARK: - some aliases for known verbs
        std::shared_ptr<Request> GET(const std::string &url) {
            return this->resource("GET", url);
        }

        std::shared_ptr<Request> POST(const std::string &url) {
            return this->resource("POST", url);
        }

        std::shared_ptr<Request> HEAD(const std::string &url) {
            return this->resource("HEAD", url);
        }

        std::shared_ptr<Request> PUT(const std::string &url) {
            return this->resource("PUT", url);
        }

        std::shared_ptr<Request> DELETE(const std::string &url) {
            return this->resource("DELETE", url);
        }

        std::shared_ptr<Request> MKCOL(const std::string &url) {
            return this->resource("MKCOL", url);
        }

        std::shared_ptr<Request> PROPFIND(const std::string &url) {
            return this->resource("PROPFIND", url);
        }

        virtual std::shared_ptr<Request> header(const std::string& key, const std::string& value) = 0;
        std::shared_ptr<Request> accept(const std::string& mimetype);
        std::shared_ptr<Request> content_type(const std::string& mimetype);
        std::shared_ptr<Request> if_match(const std::string& etag);
        virtual std::shared_ptr<Request> query_param(const std::string& key, const std::string& value) = 0;
        virtual std::shared_ptr<Request> postfield(const std::string& key, const std::string& value) = 0;
        virtual std::shared_ptr<Request> mime_postfield(const std::string& key, const std::string& value) = 0;
        virtual std::shared_ptr<Request> basic_auth(const std::string& username, const std::string& value) = 0;
        virtual std::shared_ptr<Request> token_auth(const std::string& token) = 0;

        virtual std::shared_ptr<Request> body(const std::string& body) = 0;
        virtual std::shared_ptr<Request> binary_body(const std::vector<std::uint8_t> &body) = 0;
        std::shared_ptr<Request> json_body(const nlohmann::json& json_data);

        virtual StringResponse request() = 0;
        virtual BinaryResponse request_binary() = 0;

        virtual void set_proxy(const std::string &proxyUrl, const std::string &proxyUser = "", const std::string &proxyPassword = "") = 0;

        void set_follow_redirects(bool follow);
        void set_verbose(bool verbose);

        static const std::string MIMETYPE_XML;
        static const std::string MIMETYPE_JSON;
        static const std::string MIMETYPE_BINARY;
        static const std::string MIMETYPE_TEXT;
    protected:

        bool m_option_verbose = false;
        bool m_option_follow_redirects = false;

    };
} // namespace CloudSync::request
