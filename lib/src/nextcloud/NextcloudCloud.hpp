#pragma once

#include "request/Request.hpp"
#include "pugixml.hpp"
#include "webdav/WebdavCloud.hpp"
#include <string>

using namespace pugi;
using namespace CloudSync::request;

namespace CloudSync::nextcloud {
    class NextcloudCloud : public webdav::WebdavCloud {
    public:
        NextcloudCloud(const std::string &url, const std::shared_ptr<request::Request> &request)
                : webdav::WebdavCloud(url, request) {}

        std::string getAuthorizeUrl() const override {
            return this->baseUrl + "/index.php/login/flow";
        }

        std::shared_ptr<Directory> root() const override {
            return std::make_shared<webdav::WebdavDirectory>(
                this->baseUrl, "/remote.php/webdav", "/", this->request, "");
        }

        std::string get_user_display_name() const override;

        void logout() override;
    };
} // namespace CloudSync::nextcloud
