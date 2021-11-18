#pragma once

#include "DirectoryImpl.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace CloudSync::onedrive {
    class OneDriveDirectory : public DirectoryImpl {
    public:
        OneDriveDirectory(
                const std::string &baseUrl, const std::string &dir, const std::shared_ptr<request::Request> &request,
                const std::string &name)
                : DirectoryImpl(baseUrl, dir, request, name) {};

        std::vector<std::shared_ptr<Resource>> ls() const override;

        std::shared_ptr<Directory> cd(const std::string &path) const override;

        void rmdir() const override;

        std::shared_ptr<Directory> mkdir(const std::string &path) const override;

        std::shared_ptr<File> touch(const std::string &path) const override;

        std::shared_ptr<File> file(const std::string &path) const override;

    private:
        std::shared_ptr<Resource> parseDriveItem(const json &value, const std::string &expectedType = "") const;

        std::string apiResourcePath(const std::string &path, bool children = true) const;

        std::string newResourcePath(const std::string &path) const;
    };
} // namespace CloudSync::onedrive
