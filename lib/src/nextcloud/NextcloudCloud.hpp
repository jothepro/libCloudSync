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
        NextcloudCloud(
                const std::string &url,
                const std::shared_ptr<credentials::BasicCredentialsImpl>& credentials,
                const std::shared_ptr<request::Request> &request)
                : webdav::WebdavCloud(url, credentials, request) {}

        std::shared_ptr<Directory> root() const override {
            return std::make_shared<webdav::WebdavDirectory>(
                    m_base_url, "/remote.php/webdav", "/",
                    m_credentials,
                    m_request, "");
        }

        std::string get_user_display_name() const override;

        void logout() override;
    };
}
