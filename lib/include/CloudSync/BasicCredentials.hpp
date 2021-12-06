#pragma once

#include <string>
#include <memory>

namespace CloudSync {
    /**
     * @brief Basic access authentication credentials. Used for authorization in webdav & nextcloud.
     */
    class BasicCredentials {
    public:
        virtual ~BasicCredentials() = default;
        [[nodiscard]] static std::shared_ptr<BasicCredentials> from_username_password(const std::string& username, const std::string& password);
    };
}