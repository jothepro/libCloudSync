#pragma once

#include "BoxDirectory.hpp"
#include "CloudImpl.hpp"
#include "request/Request.hpp"

namespace CloudSync::box {
    class BoxCloud : public CloudImpl {
    public:
        explicit BoxCloud(const std::shared_ptr<request::Request> &request) : CloudImpl("https://api.box.com", request) {}

        std::string getAuthorizeUrl() const override {
            return "https://account.box.com/api/oauth2/authorize";
        }

        std::string getTokenUrl() const override {
            return "https://api.box.com/oauth2/token";
        }

        std::shared_ptr<Directory> root() const override {
            return std::make_shared<BoxDirectory>("0", "0", "/", this->request, "");
        }

        static void handleExceptions(const std::exception_ptr &e, const std::string &resourcePath);

        std::string get_user_display_name() const override;

        void logout() override;
    };
} // namespace CloudSync::box
