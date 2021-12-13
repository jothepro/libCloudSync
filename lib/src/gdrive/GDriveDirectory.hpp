#pragma once

#include "OAuthDirectoryImpl.hpp"
#include <nlohmann/json.hpp>
#include <utility>

using json = nlohmann::json;

namespace CloudSync::gdrive {
class GDriveDirectory : public OAuthDirectoryImpl {
    public:
        GDriveDirectory(
                const std::string &baseUrl, std::string rootName, std::string resourceId,
                std::string parentResourceId, const std::filesystem::path &dir,
                const std::shared_ptr<credentials::OAuth2CredentialsImpl>& credentials,
                const std::shared_ptr<request::Request> &request,
                const std::string &name)
                : OAuthDirectoryImpl(baseUrl, dir, credentials, request, name)
                , m_resource_id(std::move(resourceId))
                , m_parent_resource_id(std::move(parentResourceId))
                , m_root_name(std::move(rootName)) {};

        [[nodiscard]] std::vector<std::shared_ptr<Resource>> list_resources() const override;

        [[nodiscard]] std::shared_ptr<Directory> get_directory(const std::filesystem::path &path) const override;

        void remove() override;

        std::shared_ptr<Directory> create_directory(const std::filesystem::path &path) const override;

        std::shared_ptr<File> create_file(const std::filesystem::path &path) const override;

        std::shared_ptr<File> get_file(const std::filesystem::path &path) const override;

    private:
        enum ResourceType {
            ANY, FILE, FOLDER
        };
        const std::string m_resource_id;
        const std::string m_parent_resource_id;
        const std::string m_root_name;

        std::shared_ptr<Resource> parse_file(
                const json &file, ResourceType expected_type = ResourceType::ANY,
                const std::string &custom_path = "") const;

        /// @return parent of the current directory
        std::shared_ptr<GDriveDirectory> parent() const;

        /// @return parent of the given path
        std::shared_ptr<GDriveDirectory> parent(const std::string &path, std::string &folderName, bool createIfMissing = false) const;

        std::shared_ptr<GDriveDirectory> child(const std::string &name) const;

        bool child_resource_exists(const std::string & resource_name) const;
    };
}
