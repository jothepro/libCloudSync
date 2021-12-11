#pragma once

#include "CloudSync/File.hpp"
#include "request/Request.hpp"
#include <utility>

namespace CloudSync {
    class FileImpl : public File {
    public:
        [[nodiscard]] std::string name() const override;

        [[nodiscard]] std::filesystem::path path() const override;

        [[nodiscard]] std::string revision() const override;

        [[nodiscard]] bool is_file() const override;

    protected:
        FileImpl(
                std::string baseUrl,
                std::filesystem::path dir,
                std::shared_ptr<request::Request> request,
                std::string name, std::string revision)
                : m_base_url(std::move(baseUrl))
                , m_path(std::move(dir))
                , m_request(std::move(request))
                , m_name(std::move(name))
                , m_revision(std::move(revision)) {
            assert(!m_name.empty());
            assert(m_path.generic_string().length() > 1 && m_path.generic_string()[0] == '/');
            assert(!m_revision.empty());
        };

        std::string m_revision;
        const std::string m_base_url;
        std::shared_ptr<request::Request> m_request;
        const std::string m_name;
        const std::filesystem::path m_path;
    };
}