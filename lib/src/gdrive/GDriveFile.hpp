#pragma once

#include <utility>

#include "FileImpl.hpp"

namespace CloudSync::gdrive {
    class GDriveFile : public FileImpl {
    public:
        GDriveFile(
                const std::string &baseUrl, std::string resourceId, const std::string &dir,
                const std::shared_ptr<request::Request> &request, const std::string &name, const std::string &revision)
                : FileImpl(baseUrl, dir, request, name, revision), resourceId(std::move(resourceId)) {};

        void rm() override;

        bool pollChange(bool longPoll = false) override;

        [[nodiscard]] bool supportsLongPoll() const override {
            return false;
        }

        [[nodiscard]] std::string read() const override;

        void write(const std::string &content) override;

    private:
        const std::string resourceId;
    };
} // namespace CloudSync::gdrive
