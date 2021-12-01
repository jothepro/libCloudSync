#pragma once

#include <utility>

#include "CloudImpl.hpp"
#include "request/Request.hpp"
#include "request/Response.hpp"
#include "GDriveDirectory.hpp"

namespace CloudSync::gdrive {
    class GDriveCloud : public CloudImpl {
    public:
        GDriveCloud(std::string root_name, const std::shared_ptr<request::Request> &request)
                : CloudImpl("https://www.googleapis.com/drive/v2", request), m_root_name(std::move(root_name)) {}

        std::string getAuthorizeUrl() const override {
            return "https://accounts.google.com/o/oauth2/v2/auth";
        }

        std::string getTokenUrl() const override {
            return "https://oauth2.googleapis.com/token";
        }

        std::shared_ptr<Directory> root() const override {
            return std::make_shared<GDriveDirectory>(
                    m_base_url,
                    m_root_name,
                    m_root_name,
                    m_root_name,
                    "/",
                    m_request,
                    "");
        }

        static void handleExceptions(const std::exception_ptr &e, const std::string &resourcePath);

        std::string get_user_display_name() const override;

        void logout() override;

    private:
        std::string m_root_name;
    };
} // namespace CloudSync::gdrive
