#pragma once

#include "request/Request.hpp"
#include "credentials/OAuth2CredentialsImpl.hpp"

namespace CloudSync::request::curl {

class CurlRequest : public Request, public std::enable_shared_from_this<CurlRequest> {
public:
    CurlRequest();
    ~CurlRequest() override;
    std::shared_ptr<Request> resource(const std::string &verb, const std::string &url) override;
    std::shared_ptr<Request> header(const std::string& key, const std::string& value) override;
    std::shared_ptr<Request> query_param(const std::string& key, const std::string& value) override;
    std::shared_ptr<Request> postfield(const std::string& key, const std::string& value) override;
    std::shared_ptr<Request> mime_postfield(const std::string& key, const std::string& value) override;
    std::shared_ptr<Request> basic_auth(const std::string &username, const std::string &password) override;
    std::shared_ptr<Request> token_auth(const std::string& token) override;
    std::shared_ptr<Request> body(const std::string & body) override;
    std::shared_ptr<Request> binary_body(const std::vector<std::uint8_t> &body) override;

    StringResponse request() override;
    BinaryResponse request_binary() override;

    void set_proxy(const std::string &proxy_url, const std::string &proxy_user, const std::string &proxy_password) override;
private:
    CURL *m_curl;
    std::string url_encode_param(const std::string& key, const std::string& value) const;
    void set_char_body(const char * body, size_t size);

    void prepare_request();
    void apply_proxy();
    template<typename RESPONSE_T, typename READ_T>
    RESPONSE_T perform_request();
    void cleanup_request();

    // Proxy
    std::string m_proxy_url;
    std::string m_proxy_user;
    std::string m_proxy_password;

    std::string m_verb;
    std::string m_url;
    std::string m_query_params;
    std::string m_postfields;
    struct curl_slist* m_headers = nullptr;
    curl_mime* m_form = nullptr;

    std::string m_body;
    std::vector<std::uint8_t> m_binary_body;

};

}