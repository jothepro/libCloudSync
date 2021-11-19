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

        [[nodiscard]] std::vector<std::shared_ptr<Resource>> ls() const override;

        [[nodiscard]] std::shared_ptr<Directory> cd(const std::string &path) const override;

        void rmdir() const override;

        std::shared_ptr<Directory> mkdir(const std::string &path) const override;

        std::shared_ptr<File> touch(const std::string &path) const override;

        std::shared_ptr<File> file(const std::string &path) const override;

    private:
        static std::string xmlQuery;
        const std::string dirOffset;

        std::vector<std::shared_ptr<Resource>> parseXmlResponse(const pugi::xml_node &response) const;

        /// Appends `path` to the current path and returns the result.
        std::string requestUrl(const std::string &path) const;

        static void removeTrailingSlashes(std::string &path);
    };
} // namespace CloudSync::webdav
