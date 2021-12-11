#pragma once

#include "CloudSync/Directory.hpp"
#include "request/Request.hpp"

namespace CloudSync {
    class DirectoryImpl : public Directory {
    public:
        [[nodiscard]] std::string name() const override;

        [[nodiscard]] std::filesystem::path path() const override;

        [[nodiscard]] bool is_file() const override;

    protected:
        DirectoryImpl(
                std::string baseUrl,
                std::filesystem::path dir,
                std::shared_ptr<request::Request> request,
                std::string name)
                : m_base_url(std::move(baseUrl))
                , m_path(std::move(dir))
                , m_request(std::move(request))
                , m_name(std::move(name)) {
            assert(m_path.generic_string().length() >= 1 && m_path.generic_string()[0] == '/');
            assert(m_path.generic_string() == "/" ? m_name.empty() : !m_name.empty());
        };

        std::shared_ptr<request::Request> m_request;
        const std::string m_base_url;
    protected:
        const std::string m_name;
        const std::filesystem::path m_path;

        std::filesystem::path append_path(const std::filesystem::path& child_path = "") const;
        static std::string remove_trailing_slashes(const std::string& input);
    };
}

