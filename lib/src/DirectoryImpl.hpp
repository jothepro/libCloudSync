#pragma once

#include "CloudSync/Directory.hpp"
#include "request/Request.hpp"

namespace CloudSync {
    class DirectoryImpl : public Directory {
    public:
        [[nodiscard]] std::string name() const override;

        [[nodiscard]] std::string path() const override;

        [[nodiscard]] bool is_file() const override;

    protected:
        DirectoryImpl(
                std::string baseUrl,
                std::string dir,
                std::shared_ptr<request::Request> request,
                std::string name);

        std::shared_ptr<request::Request> m_request;
        const std::string m_base_url;
    private:
        const std::string m_name;
        const std::string m_path;
    };
}

