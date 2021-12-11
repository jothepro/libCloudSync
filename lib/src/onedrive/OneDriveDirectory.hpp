#pragma once

#include "OAuthDirectoryImpl.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace CloudSync::onedrive {
    class OneDriveDirectory : public OAuthDirectoryImpl {
    public:
        OneDriveDirectory(
                const std::string &baseUrl, const std::string &dir,
                const std::shared_ptr<credentials::OAuth2CredentialsImpl>& credentials,
                const std::shared_ptr<request::Request> &request,
                const std::string &name)
                : OAuthDirectoryImpl(baseUrl, dir, credentials, request, name) {};

        [[nodiscard]] std::vector<std::shared_ptr<Resource>> list_resources() const override;

        [[nodiscard]] std::shared_ptr<Directory> get_directory(const std::filesystem::path &path) const override;

        void remove() override;

        std::shared_ptr<Directory> create_directory(const std::filesystem::path &path) const override;

        std::shared_ptr<File> create_file(const std::filesystem::path &path) const override;

        std::shared_ptr<File> get_file(const std::filesystem::path &path) const override;

    private:
        std::shared_ptr<Resource> parse_drive_item(const json &value, const std::string &expectedType = "") const;

        std::string api_resource_path(const std::string &path, bool children = true) const;
    };
} // namespace CloudSync::onedrive
