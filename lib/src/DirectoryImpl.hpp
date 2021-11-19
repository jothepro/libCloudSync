#pragma once

#include "CloudSync/Directory.hpp"

namespace CloudSync {
    class DirectoryImpl : public Directory {
    public:
        [[nodiscard]] std::string name() const override;

        [[nodiscard]] std::string path() const override;

        [[nodiscard]] std::string pwd() const override;

        [[nodiscard]] bool isFile() const override;

    protected:
        DirectoryImpl(
                std::string baseUrl,
                std::string dir,
                std::shared_ptr<request::Request> request,
                std::string name);

        std::shared_ptr<request::Request> request;
        const std::string _baseUrl;
    private:
        const std::string _name;
        const std::string _path;
    };
}

