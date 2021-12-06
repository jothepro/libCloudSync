#pragma once

#include <utility>

#include "OAuthFileImpl.hpp"

namespace CloudSync::gdrive {
    class GDriveFile : public OAuthFileImpl {
    public:
        GDriveFile(
                const std::string &baseUrl, std::string resourceId, const std::string &dir,
                const std::shared_ptr<credentials::OAuth2CredentialsImpl> credentials,
                const std::shared_ptr<request::Request> &request, const std::string &name, const std::string &revision)
                : OAuthFileImpl(baseUrl, dir, credentials, request, name, revision), resourceId(std::move(resourceId)) {};

        void remove() override;

        bool poll_change() override;

        [[nodiscard]] std::string read_as_string() const override;

        void write_string(const std::string &content) override;

    private:
        const std::string resourceId;

        [[nodiscard]] std::string resource_path() const override;
    };
} // namespace CloudSync::gdrive
