#pragma once

#include <string>
#include <memory>

namespace CloudSync {
    /// Basic access authentication credentials for authorization in WebDav & Nextcloud.
    class BasicCredentials {
    public:
        virtual ~BasicCredentials() = default;
        [[nodiscard]] static std::shared_ptr<BasicCredentials> from_username_password(const std::string& username, const std::string& password);
    };
}