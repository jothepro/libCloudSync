#pragma once

#include "OAuthFileImpl.hpp"

namespace CloudSync::onedrive {
    class OneDriveFile : public OAuthFileImpl {
    public:
        OneDriveFile(
                const std::string &baseUrl,
                const std::filesystem::path &dir,
                const std::shared_ptr<credentials::OAuth2CredentialsImpl>& credentials,
                const std::shared_ptr<request::Request> &request,
                const std::string &name,
                const std::string &revision)
                : OAuthFileImpl(baseUrl, dir, credentials, request, name, revision)
                , m_resource_path(m_base_url + ":" + m_path.generic_string()){};

        void remove() override;

        bool poll_change() override;

        [[nodiscard]] std::string read() const override;
        [[nodiscard]] std::vector<std::uint8_t> read_binary() const override;

        void write(const std::string& content) override;
        void write_binary(const std::vector<std::uint8_t> & content) override;
    private:
        const std::string m_resource_path;

        std::shared_ptr<request::Request> prepare_read_request() const;
        std::shared_ptr<request::Request> prepare_write_request() const;
    };
} // namespace CloudSync::onedrive
