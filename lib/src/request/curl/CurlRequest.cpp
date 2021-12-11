#include "CurlRequest.hpp"
#include "credentials/OAuth2CredentialsImpl.hpp"
#include "request/exceptions/RequestException.hpp"

namespace CloudSync::request::curl {
    CurlRequest::CurlRequest() {
        m_curl = curl_easy_init();
    }

    CurlRequest::~CurlRequest() {
        curl_easy_cleanup(m_curl);
    }

    static size_t BinaryWriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
        std::vector<std::uint8_t> new_contents(
                (std::uint8_t *) contents,
                (std::uint8_t *) contents + size * nmemb);
        ((std::vector<std::uint8_t> *) userp)->insert(
                ((std::vector<std::uint8_t> *) userp)->end(),
                new_contents.begin(),
                new_contents.end());
        return size * nmemb;
    }

    static size_t StringWriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
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

    std::shared_ptr<Request> CurlRequest::resource(const std::string &verb, const std::string &url) {
        // make sure that the resource call is the first one in a request
        assert(m_verb.empty());
        assert(m_url.empty());
        assert(m_query_params.empty());
        assert(m_postfields.empty());
        assert(m_form == nullptr);
        assert(m_headers == nullptr);

        m_verb = verb;
        m_url = url;
        return this->shared_from_this();
    }

    std::shared_ptr<Request> CurlRequest::header(const std::string &key, const std::string &value) {
        m_headers = curl_slist_append(m_headers, (key + ": " + value).c_str());
        return this->shared_from_this();
    }

    std::shared_ptr<Request> CurlRequest::query_param(const std::string &key, const std::string &value) {
        m_query_params += m_query_params.empty() ? "?" : "&";
        m_query_params += url_encode_param(key, value);
        return this->shared_from_this();
    }

    std::shared_ptr<Request> CurlRequest::postfield(const std::string &key, const std::string &value) {
        if(!m_postfields.empty()) {
            m_postfields += "&";
        }
        m_postfields += url_encode_param(key, value);
        return this->shared_from_this();
    }

    std::shared_ptr<Request> CurlRequest::mime_postfield(const std::string &key, const std::string &value) {
        if(m_form == nullptr) {
            m_form = curl_mime_init(m_curl);
        }
        curl_mimepart *mime_part = curl_mime_addpart(m_form);
        curl_mime_name(mime_part, key.c_str());
        curl_mime_data(mime_part, value.c_str(), value.size());
        return this->shared_from_this();
    }

    StringResponse CurlRequest::request() {
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, StringWriteCallback);
        return perform_request<StringResponse, std::string>();
    }

    BinaryResponse CurlRequest::request_binary() {
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, BinaryWriteCallback);
        return perform_request<BinaryResponse, std::vector<std::uint8_t>>();
    }

    void CurlRequest::prepare_request() {
        if (m_option_follow_redirects) {
            curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1);
        }

        if (m_option_verbose) {
            curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1);
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
            }
        }
    }

    void CurlRequest::apply_proxy() {
        if (!m_proxy_url.empty()) {
            curl_easy_setopt(m_curl, CURLOPT_PROXY, m_proxy_url.c_str());
            if (!m_proxy_user.empty() && !m_proxy_password.empty()) {
                const auto escaped_proxy_user = curl_easy_escape(
                        m_curl,
                        m_proxy_user.c_str(),
                        m_proxy_user.size());
                const auto escaped_proxy_password = curl_easy_escape(
                        m_curl,
                        m_proxy_password.c_str(),
                        m_proxy_password.size());
                const auto m_proxy_user_pwd = std::string(escaped_proxy_user) + ":" + std::string(escaped_proxy_password);
                curl_free(escaped_proxy_user);
                curl_free(escaped_proxy_password);
                curl_easy_setopt(m_curl, CURLOPT_PROXYUSERPWD, m_proxy_user_pwd.c_str());
            }
        }
    }

    template<typename RESPONSE_T, typename READ_T>
    RESPONSE_T CurlRequest::perform_request() {
        assert(!m_verb.empty());
        assert(!m_url.empty());

        prepare_request();
        apply_proxy();

        // perform request
        READ_T response_read_buffer;
        std::unordered_map<std::string, std::string> response_headers;
        long response_code;
        char *response_content_type;
        char error_buffer[CURL_ERROR_SIZE];
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &response_read_buffer);
        curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, &response_headers);
        curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, error_buffer);
        const auto request_result = curl_easy_perform(m_curl);
        const auto response_code_info_result = curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &response_code);
        const auto content_type_info_result = curl_easy_getinfo(m_curl, CURLINFO_CONTENT_TYPE, &response_content_type);
        // when the response has no body, response_content_type is a nullptr. This needs to be checked when
        // transforming the char* to a string.
        const std::string response_content_type_string = response_content_type ? std::string(response_content_type) : "";
        // return result
        if (request_result == CURLE_OK && response_code_info_result == CURLE_OK && content_type_info_result == CURLE_OK) {
            cleanup_request();
            return RESPONSE_T(response_code, response_read_buffer, response_content_type_string, response_headers);
        } else {
            const auto error_message = std::string(error_buffer);
            cleanup_request();
            throw request::exceptions::RequestException(error_message);
        }
    }

    void CurlRequest::cleanup_request() {
        curl_mime_free(m_form);
        m_form = nullptr;
        curl_slist_free_all(m_headers);
        m_headers = nullptr;
        m_query_params.clear();
        m_postfields.clear();
        m_verb.clear();
        m_url.clear();
        m_body.clear();
        m_binary_body.clear();
        curl_easy_reset(m_curl);
    }

    std::string CurlRequest::url_encode_param(const std::string& key, const std::string& value) const {
        std::string result;
        const auto escaped_key = curl_easy_escape(m_curl, key.c_str(), key.size());
        const auto escaped_value = curl_easy_escape(m_curl, value.c_str(), value.size());
        result += std::string(escaped_key) + "=" + std::string(escaped_value);
        curl_free(escaped_key);
        curl_free(escaped_value);
        return result;
    }

    void CurlRequest::set_proxy(const std::string &proxy_url, const std::string &proxy_user,
                                const std::string &proxy_password) {
        m_proxy_url = proxy_url;
        m_proxy_user = proxy_user;
        m_proxy_password = proxy_password;
    }

    std::shared_ptr<Request> CurlRequest::basic_auth(const std::string &username, const std::string &password) {
        curl_easy_setopt(m_curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(m_curl, CURLOPT_USERNAME, username.c_str());
        curl_easy_setopt(m_curl, CURLOPT_PASSWORD, password.c_str());
        return this->shared_from_this();
    }

    std::shared_ptr<Request> CurlRequest::token_auth(const std::string &token) {
        curl_easy_setopt(m_curl, CURLOPT_HTTPAUTH, CURLAUTH_BEARER);
        curl_easy_setopt(m_curl, CURLOPT_XOAUTH2_BEARER, token.c_str());
        return this->shared_from_this();
    }

    std::shared_ptr<Request> CurlRequest::binary_body(const std::vector<std::uint8_t> &body) {
        m_binary_body = body;
        set_char_body(reinterpret_cast<const char *>(m_binary_body.data()), m_binary_body.size());
        return this->shared_from_this();
    }

    std::shared_ptr<Request> CurlRequest::body(const std::string &body) {
        m_body = body;
        set_char_body(m_body.data(), m_body.size());
        return this->shared_from_this();
    }

    void CurlRequest::set_char_body(const char *body, const size_t size) {
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, body);
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, size);
    }

}

