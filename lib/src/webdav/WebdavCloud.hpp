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
            return std::make_shared<WebdavDirectory>(this->baseUrl, "", "/", this->request, "");
        }

        static void handleExceptions(const std::exception_ptr &e, const std::string &resourcePath);

        std::string get_user_display_name() const override {
            return this->request->getUsername();
        };

        void logout() override;
    };
} // namespace CloudSync::webdav
