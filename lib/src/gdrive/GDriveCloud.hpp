#pragma once

#include "CloudImpl.hpp"
#include "request/Request.hpp"
#include "request/Response.hpp"
#include "GDriveDirectory.hpp"

namespace CloudSync::gdrive {
    class GDriveCloud : public CloudImpl {
    public:
        GDriveCloud(const std::string &rootName, const std::shared_ptr<request::Request> &request)
                : CloudImpl("https://www.googleapis.com/drive/v2", request), rootName(rootName) {}

        std::string getAuthorizeUrl() const override {
            return "https://accounts.google.com/o/oauth2/v2/auth";
        }

        std::string getTokenUrl() const override {
            return "https://oauth2.googleapis.com/token";
        }

        std::shared_ptr<Directory> root() const override {
            return std::make_shared<GDriveDirectory>(
                    this->baseUrl,
                    this->rootName,
                    this->rootName,
                    this->rootName,
                    "/",
                    this->request,
                    "");
        }

        static void handleExceptions(const std::exception_ptr &e, const std::string &resourcePath);

        std::string getUserDisplayName() const override;

        void logout() override;

    private:
        std::string rootName;
    };
} // namespace CloudSync::gdrive
