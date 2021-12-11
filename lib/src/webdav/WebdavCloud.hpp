#pragma once

#include "CloudImpl.hpp"
#include "request/Request.hpp"
#include "WebdavDirectory.hpp"
#include "credentials/BasicCredentialsImpl.hpp"
#include <algorithm>
#include <utility>

namespace CloudSync::webdav {
    class WebdavCloud : public CloudImpl {
    public:
        WebdavCloud(
                const std::string &url,
                std::shared_ptr<credentials::BasicCredentialsImpl>  credentials,
                const std::shared_ptr<request::Request> &request)
                : CloudImpl(url,request), m_credentials(std::move(credentials)) {}

        std::shared_ptr<Directory> root() const override;

        std::string get_user_display_name() const override;

        void logout() override;

    protected:
        const std::shared_ptr<credentials::BasicCredentialsImpl> m_credentials;
    };
} // namespace CloudSync::webdav
