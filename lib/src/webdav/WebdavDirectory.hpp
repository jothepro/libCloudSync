#pragma once

#include "DirectoryImpl.hpp"
#include "credentials/BasicCredentialsImpl.hpp"

#include <utility>
#include "request/Response.hpp"

namespace CloudSync::webdav {
    class WebdavDirectory : public DirectoryImpl {
    public:
        WebdavDirectory(
                const std::string &baseUrl, std::string dirOffset, const std::filesystem::path &dir,
                std::shared_ptr<credentials::BasicCredentialsImpl> credentials,
                const std::shared_ptr<request::Request> &request, const std::string &name)
                : DirectoryImpl(baseUrl, dir, request, name)
                , m_credentials(std::move(credentials))
                , m_dir_offset(std::move(dirOffset)) {};

        [[nodiscard]] std::vector<std::shared_ptr<Resource>> list_resources() const override;

        [[nodiscard]] std::shared_ptr<Directory> get_directory(const std::filesystem::path &path) const override;

        void remove() override;

        std::shared_ptr<Directory> create_directory(const std::filesystem::path &path) const override;

        std::shared_ptr<File> create_file(const std::filesystem::path &path) const override;

        std::shared_ptr<File> get_file(const std::filesystem::path &path) const override;

    private:
        static const std::string XML_QUERY;
        const std::string m_dir_offset;

        const std::shared_ptr<credentials::BasicCredentialsImpl> m_credentials;

        [[nodiscard]] std::vector<std::shared_ptr<Resource>> parse_xml_response(const pugi::xml_node &response) const;

        bool resource_exists(const std::filesystem::path& resource_path) const;

        void create_parent_directories_if_missing(const std::filesystem::path& absolute_path) const;
    };
} // namespace CloudSync::webdav
