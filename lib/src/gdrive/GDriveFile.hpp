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

        [[nodiscard]] std::string read() const override;

        [[nodiscard]] std::vector<std::uint8_t> read_binary() const override;

        void write(const std::string& content) override;

        void write_binary(const std::vector<std::uint8_t>& content) override;

    private:
        const std::string resourceId;

        const std::string m_resource_path = m_base_url + "/files/" + this->resourceId;
    };
} // namespace CloudSync::gdrive
