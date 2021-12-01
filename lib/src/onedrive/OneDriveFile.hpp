#pragma once

#include "FileImpl.hpp"

namespace CloudSync::onedrive {
    class OneDriveFile : public FileImpl {
    public:
        OneDriveFile(
                const std::string &baseUrl, const std::string &dir, const std::shared_ptr<request::Request> &request,
                const std::string &name, const std::string &revision)
                : FileImpl(baseUrl, dir, request, name, revision) {};

        void remove() override;

        bool poll_change() override;

        [[nodiscard]] std::string read_as_string() const override;

        void write_string(const std::string &content) override;
    private:
        [[nodiscard]] std::string resource_path() const override;
    };
} // namespace CloudSync::onedrive
