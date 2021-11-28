#pragma once

#include "DirectoryImpl.hpp"
#include "request/Response.hpp"
#include <nlohmann/json.hpp>
#include <utility>

using json = nlohmann::json;

namespace CloudSync::box {
    class BoxDirectory : public DirectoryImpl {
    public:
        BoxDirectory(
                std::string resourceId, std::string parentResourceId, const std::string &dir,
                const std::shared_ptr<request::Request> &request, const std::string &name)
                : DirectoryImpl("", dir, request, name), resourceId(std::move(resourceId)), parentResourceId(std::move(parentResourceId)) {};

        std::vector<std::shared_ptr<Resource>> list_resources() const override;

        std::shared_ptr<Directory> get_directory(const std::string &path) const override;

        void remove() override;

        std::shared_ptr<Directory> create_directory(const std::string &path) const override;

        std::shared_ptr<File> create_file(const std::string &path) const override;

        std::shared_ptr<File> get_file(const std::string &path) const override;

    private:
        const std::string resourceId;
        const std::string parentResourceId;

        std::shared_ptr<Resource>
        parseEntry(const json &entry, const std::string &expectedType = "", const std::string &customPath = "") const;

        /// @return parent of the current directory
        std::shared_ptr<BoxDirectory> parent() const;

        /// @return parent of the given path
        std::shared_ptr<BoxDirectory> parent(const std::string &path, std::string &folderName, bool createIfMissing = false) const;

        std::shared_ptr<BoxDirectory> child(const std::string &name) const;
    };
} // namespace CloudSync::box
