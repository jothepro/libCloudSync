#pragma once

#include "FileImpl.hpp"
#include "CloudSync/OAuth2Credentials.hpp"
#include "credentials/OAuth2CredentialsImpl.hpp"
#include <utility>

namespace CloudSync {
    class OAuthFileImpl : public FileImpl {
    protected:
        OAuthFileImpl(std::string baseUrl,
                      std::filesystem::path dir,
                      std::shared_ptr<credentials::OAuth2CredentialsImpl> credentials,
                      std::shared_ptr<request::Request> request,
                      std::string name, std::string revision)
                      : FileImpl(std::move(baseUrl), std::move(dir), std::move(request), std::move(name), std::move(revision))
                      , m_credentials(std::move(credentials)) {};
        const std::shared_ptr<credentials::OAuth2CredentialsImpl> m_credentials;
    };
}