#pragma once

#include "CloudSync/Directory.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace CloudSync::gdrive {
class GDriveDirectory : public Directory {
  public:
    GDriveDirectory(
        const std::string &baseUrl, const std::string &rootName, const std::string &resourceId,
        const std::string &parentResourceId, const std::string &dir, const std::shared_ptr<request::Request> &request,
        const std::string &name)
        : Directory(baseUrl, dir, request, name), resourceId(resourceId), parentResourceId(parentResourceId),
          rootName(rootName){};
    std::vector<std::shared_ptr<Resource>> ls() const override;
    std::shared_ptr<Directory> cd(const std::string &path) const override;
    void rmdir() const override;
    std::shared_ptr<Directory> mkdir(const std::string &path) const override;
    std::shared_ptr<File> touch(const std::string &path) const override;
    std::shared_ptr<File> file(const std::string &path) const override;

  private:
    enum ResourceType { ANY, FILE, FOLDER };
    const std::string resourceId;
    const std::string parentResourceId;
    const std::string rootName;
    std::shared_ptr<Resource> parseFile(
        const json &file, ResourceType expectedType = ResourceType::ANY, const std::string &customPath = "") const;

    /// @return parent of the current directory
    std::shared_ptr<GDriveDirectory> parent() const;
    /// @return parent of the given path
    std::shared_ptr<GDriveDirectory> parent(const std::string &path, std::string &folderName) const;
    std::shared_ptr<GDriveDirectory> child(const std::string &name) const;
};
} // namespace CloudSync::gdrive
