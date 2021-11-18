#pragma once

#include "DirectoryImpl.hpp"
#include "request/Response.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace CloudSync::box {
    class BoxDirectory : public DirectoryImpl {
    public:
        BoxDirectory(
                const std::string &resourceId, const std::string &parentResourceId, const std::string &dir,
                const std::shared_ptr<request::Request> &request, const std::string &name)
                : DirectoryImpl("", dir, request, name), resourceId(resourceId), parentResourceId(parentResourceId) {};

        std::vector<std::shared_ptr<Resource>> ls() const override;

        std::shared_ptr<Directory> cd(const std::string &path) const override;

        void rmdir() const override;

        std::shared_ptr<Directory> mkdir(const std::string &path) const override;

        std::shared_ptr<File> touch(const std::string &path) const override;

        std::shared_ptr<File> file(const std::string &path) const override;

    private:
        const std::string resourceId;
        const std::string parentResourceId;

        std::shared_ptr<Resource>
        parseEntry(const json &entry, const std::string &expectedType = "", const std::string &customPath = "") const;

        /// @return parent of the current directory
        std::shared_ptr<BoxDirectory> parent() const;

        /// @return parent of the given path
        std::shared_ptr<BoxDirectory> parent(const std::string &path, std::string &folderName) const;

        std::shared_ptr<BoxDirectory> child(const std::string &name) const;
    };
} // namespace CloudSync::box
