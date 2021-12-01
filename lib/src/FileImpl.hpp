#pragma once

#include <utility>

#include "CloudSync/File.hpp"

namespace CloudSync {
    class FileImpl : public File {
    public:
        [[nodiscard]] std::string name() const override;

        [[nodiscard]] std::string path() const override;

        [[nodiscard]] std::string revision() const override;

        [[nodiscard]] bool is_file() const override;

    protected:
        FileImpl(
                std::string baseUrl,
                std::string dir,
                std::shared_ptr<request::Request> request,
                std::string name, std::string revision)
                : m_base_url(std::move(baseUrl)), m_path(std::move(dir)), m_request(std::move(request)),
                  m_name(std::move(name)), m_revision(std::move(revision)) {};

        std::string m_revision;
        const std::string m_base_url;
        std::shared_ptr<request::Request> m_request;
    protected:
        [[nodiscard]] virtual std::string resource_path() const = 0;
    private:
        const std::string m_name;
        const std::string m_path;
    };
}