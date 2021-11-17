#pragma once

#include "CloudSync/File.hpp"

namespace CloudSync::onedrive {
class OneDriveFile : public File {
  public:
    OneDriveFile(
        const std::string &baseUrl, const std::string &dir, const std::shared_ptr<request::Request> &request,
        const std::string &name, const std::string &revision)
        : File(baseUrl, dir, request, name, revision){};
    void rm() override;
    bool pollChange(bool longPoll = false) override;
    bool supportsLongPoll() const override {
        return false;
    }
    std::string read() const override;
    void write(const std::string &content) override;
};
} // namespace CloudSync::onedrive
