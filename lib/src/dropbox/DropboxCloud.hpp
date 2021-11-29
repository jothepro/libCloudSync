#pragma once

#include "CloudImpl.hpp"
#include "request/Request.hpp"
#include "DropboxDirectory.hpp"

using namespace CloudSync::request;

namespace CloudSync::dropbox {
    class DropboxCloud : public CloudImpl {
    public:
        explicit DropboxCloud(const std::shared_ptr<request::Request> &request) : CloudImpl("https://www.dropbox.com",
                                                                                   request) {}

        std::string getAuthorizeUrl() const override {
            return "https://www.dropbox.com/oauth2/authorize";
        }

        std::string getTokenUrl() const override {
            return "https://api.dropboxapi.com/oauth2/token";
        }

        std::shared_ptr<Directory> root() const override {
            return std::make_shared<DropboxDirectory>("/", this->request, "");
        }

        static void handleExceptions(const std::exception_ptr &e, const std::string &resourcePath);

        std::string get_user_display_name() const override;

        void logout() override;
    };
} // namespace CloudSync::dropbox
