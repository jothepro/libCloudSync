#pragma once

#include "DirectoryImpl.hpp"

#include <utility>
#include "request/Response.hpp"

namespace CloudSync::webdav {
    class WebdavDirectory : public DirectoryImpl {
    public:
        WebdavDirectory(
                const std::string &baseUrl, std::string dirOffset, const std::string &dir,
                const std::shared_ptr<request::Request> &request, const std::string &name)
                : DirectoryImpl(baseUrl, dir, request, name), dirOffset(std::move(dirOffset)) {};

        [[nodiscard]] std::vector<std::shared_ptr<Resource>> list_resources() const override;

        [[nodiscard]] std::shared_ptr<Directory> get_directory(const std::string &path) const override;

        void remove() override;

        std::shared_ptr<Directory> create_directory(const std::string &path) const override;

        std::shared_ptr<File> create_file(const std::string &path) const override;

        std::shared_ptr<File> get_file(const std::string &path) const override;

    private:
        static std::string xmlQuery;
        const std::string dirOffset;

        std::vector<std::shared_ptr<Resource>> parseXmlResponse(const pugi::xml_node &response) const;

        bool resource_exists(const std::string& resource_path) const;

        /// Appends `path` to the current path and returns the result.
        std::string requestUrl(const std::string &path) const;

        static std::string remove_trailing_slashes(const std::string &path);
    };
} // namespace CloudSync::webdav
