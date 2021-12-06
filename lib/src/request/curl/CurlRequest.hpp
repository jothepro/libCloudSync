#pragma once

#include "request/Request.hpp"
#include "credentials/OAuth2CredentialsImpl.hpp"

namespace CloudSync::request::curl {

class CurlRequest : public Request, public std::enable_shared_from_this<CurlRequest> {
public:
    CurlRequest();
    ~CurlRequest() override;
    std::shared_ptr<Request> request(const std::string &verb, const std::string &url) override;
    std::shared_ptr<Request> header(const std::string& key, const std::string& value) override;
    std::shared_ptr<Request> query_param(const std::string& key, const std::string& value) override;
    std::shared_ptr<Request> postfield(const std::string& key, const std::string& value) override;
    std::shared_ptr<Request> mime_postfield(const std::string& key, const std::string& value) override;
    std::shared_ptr<Request> mime_postfile(const std::string& key, const std::string& value) override;
    std::shared_ptr<Request> basic_auth(const std::string &username, const std::string &password) override;
    std::shared_ptr<Request> token_auth(const std::string& token) override;
    Response send(const std::optional<std::string>& body = std::nullopt) override;

    void set_proxy(const std::string &proxyUrl, const std::string &proxyUser, const std::string &proxyPassword) override;
private:
    CURL *m_curl;
    std::string url_encode_param(const std::string& key, const std::string& value) const;

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
};

}