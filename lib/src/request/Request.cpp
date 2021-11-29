#include "Request.hpp"
#include <chrono>

using namespace std::chrono;

namespace CloudSync::request {
    Request::Request() {
        m_curl = curl_easy_init();
    }

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
        ((std::string *) userp)->append((char *) contents, size * nmemb);
        return size * nmemb;
    }

    static size_t HeaderCallback(char *contents, size_t size, size_t nmemb, void *userp) {
        const auto whitespace = " \n\r";
        const std::string header = std::string((char *) contents, size * nmemb);
        const auto separator_position = header.find_first_of(':');
        if (separator_position != std::string::npos) {
            std::string key = header.substr(0, separator_position);
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            const std::string value = header.substr(separator_position + 1);
            // trim away unwanted leading and trailing whitespaces
            size_t start = value.find_first_not_of(whitespace);
            size_t end = value.find_last_not_of(whitespace);
            const std::string trimmed_value = value.substr(start, (end - start) + 1);
            ((std::unordered_map<std::string, std::string> *) userp)->insert({key, trimmed_value});
        }
        return size * nmemb;
    }

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
            m_refresh_token = "";
            m_access_token = "";
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
        m_access_token = "";
        m_refresh_token = "";
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
        m_username = "";
        m_password = "";
    }

    void
    Request::set_proxy(const std::string &proxyUrl, const std::string &proxyUser, const std::string &proxyPassword) {
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
        m_access_token = "";
        m_refresh_token = "";
        m_username = "";
        m_password = "";
    }

    std::shared_ptr<Request> Request::request(const std::string &verb, const std::string &url) {
        m_verb = verb;
        m_url = url;
        return this->shared_from_this();
    }

    Response Request::send(const std::string& body) {
        if (!m_token_request_url.empty() && (!m_access_token.empty() || !m_refresh_token.empty())) {
            refresh_oauth2_token_if_needed();
        }
        // enable redirects
        if (m_option_follow_redirects)
            curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1);

        // set verbose
        if (m_option_verbose)
            curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1);

        // authorization
        if (!m_username.empty() && !m_password.empty()) {
            // Basic Auth
            curl_easy_setopt(m_curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            curl_easy_setopt(m_curl, CURLOPT_USERNAME, m_username.c_str());
            curl_easy_setopt(m_curl, CURLOPT_PASSWORD, m_password.c_str());
        } else if (!m_token_request_url.empty() && (!m_access_token.empty() || !m_refresh_token.empty())) {
            // OAuth2
            m_headers = curl_slist_append(m_headers, std::string("Authorization: Bearer " + m_access_token).c_str());
        }

        // apply proxy
        if (!m_proxy_url.empty()) {
            curl_easy_setopt(m_curl, CURLOPT_PROXY, m_proxy_url.c_str());
            if (!m_proxy_user.empty() && !m_proxy_password.empty()) {
                const auto escaped_proxy_user = curl_easy_escape(
                        m_curl,
                        m_proxy_user.c_str(),
                        m_proxy_user.length());
                const auto escaped_proxy_password = curl_easy_escape(
                        m_curl,
                        m_proxy_password.c_str(),
                        m_proxy_password.length());
                const auto m_proxy_user_pwd = std::string(escaped_proxy_user) + ":" + std::string(escaped_proxy_password);
                curl_free(escaped_proxy_user);
                curl_free(escaped_proxy_password);
                curl_easy_setopt(m_curl, CURLOPT_PROXYUSERPWD, m_proxy_user_pwd.c_str());
            }
        }
        // set url (including potential query params)
        auto final_url = m_url + m_query_params;
        curl_easy_setopt(m_curl, CURLOPT_URL, final_url.c_str());

        // set headers
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_headers);

        if (m_verb != "GET") {
            if (m_verb == "HEAD") {
                curl_easy_setopt(m_curl, CURLOPT_NOBODY, 1);
            } else {
                curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, m_verb.c_str());
            }
            if (!m_postfields.empty()) {
                curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, m_postfields.c_str());
            } else if (m_form != nullptr) {
                curl_easy_setopt(m_curl, CURLOPT_MIMEPOST, m_form);
            } else {
                curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, body.c_str());
            }
        }

        // perform request
        std::string response_read_buffer;
        std::unordered_map<std::string, std::string> response_headers;
        long response_code;
        char *response_content_type;
        char error_buffer[CURL_ERROR_SIZE];
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &response_read_buffer);
        curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, &response_headers);
        curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, error_buffer);
        const auto request_result = curl_easy_perform(m_curl);
        const auto response_code_info_result = curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &response_code);
        const auto content_type_info_result = curl_easy_getinfo(m_curl, CURLINFO_CONTENT_TYPE, &response_content_type);

        // cleanup after request
        curl_mime_free(m_form);
        m_form = nullptr;
        curl_slist_free_all(m_headers);
        m_headers = nullptr;
        curl_easy_reset(m_curl);
        m_query_params = "";
        m_postfields = "";

        // return result
        if (request_result == CURLE_OK && response_code_info_result == CURLE_OK && content_type_info_result == CURLE_OK) {
            // when the response has no body, responseContentType is a nullptr. This needs to be checked when
            // transforming the char* to a string.
            const std::string response_content_type_string = response_content_type ? std::string(response_content_type) : "";
            auto response = Response(response_code, response_read_buffer, response_content_type_string, response_headers);
            return response;
        } else {
            const auto errorMessage = std::string(error_buffer);
            throw RequestException(errorMessage);
        }
    }

    Request::~Request() {
        curl_easy_cleanup(m_curl);
    }

    std::shared_ptr<Request> Request::query_param(const std::string &key, const std::string &value) {
        if(m_query_params.empty()) {
            m_query_params += "?";
        } else {
            m_query_params += "&";
        }
        m_query_params += url_encode_param(key, value);
        return this->shared_from_this();
    }

    std::shared_ptr<Request> Request::header(const std::string &key, const std::string &value) {
        m_headers = curl_slist_append(m_headers, (key + ": " + value).c_str());
        return this->shared_from_this();
    }

    std::string Request::url_encode_param(const std::string& key, const std::string& value) const {
        std::string result;
        const auto escaped_key = curl_easy_escape(m_curl, key.c_str(), key.size());
        const auto escaped_value = curl_easy_escape(m_curl, value.c_str(), value.size());
        result += std::string(escaped_key) + "=" + std::string(escaped_value);
        curl_free(escaped_key);
        curl_free(escaped_value);
        return result;
    }

    std::shared_ptr<Request> Request::postfield(const std::string &key, const std::string &value) {
        if(!m_postfields.empty()) {
            m_postfields += "&";
        }
        m_postfields += Request::url_encode_param(key, value);
        return this->shared_from_this();
    }

    std::shared_ptr<Request> Request::mime_postfield(const std::string &key, const std::string &value) {
        if(m_form == nullptr) {
            m_form = curl_mime_init(m_curl);
        }
        curl_mimepart *mime_part = curl_mime_addpart(m_form);
        curl_mime_name(mime_part, key.c_str());
        curl_mime_data(mime_part, value.c_str(), CURL_ZERO_TERMINATED);
        return this->shared_from_this();
    }

    std::shared_ptr<Request> Request::mime_postfile(const std::string &key, const std::string &value) {
        if(m_form == nullptr) {
            m_form = curl_mime_init(m_curl);
        }
        curl_mimepart *mime_part = curl_mime_addpart(m_form);
        curl_mime_filename(mime_part, "upload");
        curl_mime_name(mime_part, key.c_str());
        curl_mime_data(mime_part, value.c_str(), CURL_ZERO_TERMINATED);
        return this->shared_from_this();
    }

    Response Request::send_json(const nlohmann::json &json_data) {
        content_type(Request::MIMETYPE_JSON);
        return send(json_data.dump());
    }

    std::shared_ptr<Request> Request::accept(const std::string &mimetype) {
        header("Accept", mimetype);
        return this->shared_from_this();
    }

    std::shared_ptr<Request> Request::content_type(const std::string &mimetype) {
        header("Content-Type", mimetype);
        return this->shared_from_this();
    }

    std::shared_ptr<Request> Request::if_match(const std::string &etag) {
        header("If-Match", etag);
        return this->shared_from_this();
    }

} // namespace CloudSync::request
