#pragma once

#include "CloudImpl.hpp"
#include "credentials/OAuth2CredentialsImpl.hpp"
#include <utility>

namespace CloudSync {
    class OAuthCloudImpl : public CloudImpl {

    protected:
        OAuthCloudImpl(
                std::string url,
                const std::string& token_endpoint,
                std::shared_ptr<credentials::OAuth2CredentialsImpl>  credentials,
                const std::shared_ptr<request::Request>& request)
                : CloudImpl(std::move(url), request)
                , m_credentials(std::move(credentials)) {
                    m_credentials->set_request(request);
                    m_credentials->set_token_endpoint(token_endpoint);
                };
        const std::shared_ptr<credentials::OAuth2CredentialsImpl> m_credentials;
    };
}