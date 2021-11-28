#pragma once

#include "DirectoryImpl.hpp"
#include <nlohmann/json.hpp>
#include <utility>

using json = nlohmann::json;

namespace CloudSync::gdrive {
class GDriveDirectory : public DirectoryImpl {
    public:
        GDriveDirectory(
                const std::string &baseUrl, std::string rootName, std::string resourceId,
                std::string parentResourceId, const std::string &dir,
                const std::shared_ptr<request::Request> &request,
                const std::string &name)
                : DirectoryImpl(baseUrl, dir, request, name)
                , resourceId(std::move(resourceId))
                , parentResourceId(std::move(parentResourceId))
                , rootName(std::move(rootName)) {};

        [[nodiscard]] std::vector<std::shared_ptr<Resource>> list_resources() const override;

        [[nodiscard]] std::shared_ptr<Directory> get_directory(const std::string &path) const override;

        void remove() override;

        std::shared_ptr<Directory> create_directory(const std::string &path) const override;

        std::shared_ptr<File> create_file(const std::string &path) const override;

        std::shared_ptr<File> get_file(const std::string &path) const override;

    private:
        enum ResourceType {
            ANY, FILE, FOLDER
        };
        const std::string resourceId;
        const std::string parentResourceId;
        const std::string rootName;

        std::shared_ptr<Resource> parseFile(
                const json &file, ResourceType expectedType = ResourceType::ANY,
                const std::string &customPath = "") const;

        /// @return parent of the current directory
        std::shared_ptr<GDriveDirectory> parent() const;

        /// @return parent of the given path
        std::shared_ptr<GDriveDirectory> parent(const std::string &path, std::string &folderName, bool createIfMissing = false) const;

        std::shared_ptr<GDriveDirectory> child(const std::string &name) const;

        bool child_resource_exists(const std::string & resource_name) const;

        std::shared_ptr<GDriveDirectory> get_or_create_parent_directory(const std::string& path) const;
    };
} // namespace CloudSync::gdrive
