#pragma once

#include <utility>

#include "CloudSync/File.hpp"

namespace CloudSync {
    class FileImpl : public File {
    public:
        [[nodiscard]] std::string name() const override;

        [[nodiscard]] std::string path() const override;

        [[nodiscard]] std::string revision() const override;

        [[nodiscard]] bool isFile() const override;

    protected:
        FileImpl(
                std::string baseUrl,
                std::string dir,
                std::shared_ptr<request::Request> request,
                std::string name, std::string revision)
                : _baseUrl(std::move(baseUrl)), _path(std::move(dir)), request(std::move(request)),
                  _name(std::move(name)), _revision(std::move(revision)) {};

        std::string _revision;
        const std::string _baseUrl;
        std::shared_ptr<request::Request> request;
    private:
        const std::string _name;
        const std::string _path;
    };
}