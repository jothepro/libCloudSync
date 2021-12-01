#pragma once

#include "CloudImpl.hpp"
#include "CloudSync/Exceptions.hpp"
#include "request/Request.hpp"
#include "WebdavDirectory.hpp"
#include <algorithm>

namespace CloudSync::webdav {
    class WebdavCloud : public CloudImpl {
    public:
        WebdavCloud(const std::string &url, const std::shared_ptr<request::Request> &request) : CloudImpl(url,
                                                                                                          request) {}

        std::string getAuthorizeUrl() const override {
            return "";
        }

        std::string getTokenUrl() const override {
            return "";
        }

        std::shared_ptr<Directory> root() const override {
            return std::make_shared<WebdavDirectory>(m_base_url, "", "/", m_request, "");
        }

        static void handleExceptions(const std::exception_ptr &e, const std::string &resourcePath);

        std::string get_user_display_name() const override {
            return m_request->get_username();
        };

        void logout() override;
    };
} // namespace CloudSync::webdav
