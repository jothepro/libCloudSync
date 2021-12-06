#pragma once

#include "DirectoryImpl.hpp"
#include "credentials/OAuth2CredentialsImpl.hpp"
#include <utility>

namespace CloudSync {
    class OAuthDirectoryImpl : public DirectoryImpl {
    protected:
        OAuthDirectoryImpl(
                std::string baseUrl,
                std::string dir,
                std::shared_ptr<credentials::OAuth2CredentialsImpl> credentials,
                std::shared_ptr<request::Request> request,
                std::string name)
                : DirectoryImpl(std::move(baseUrl), std::move(dir), std::move(request), std::move(name))
                , m_credentials(std::move(credentials)) {};
        const std::shared_ptr<credentials::OAuth2CredentialsImpl> m_credentials;
    };
}