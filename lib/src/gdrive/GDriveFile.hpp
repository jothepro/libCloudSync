#pragma once

#include "CloudSync/File.hpp"

namespace CloudSync::gdrive {
class GDriveFile : public File {
  public:
    GDriveFile(
        const std::string &baseUrl, const std::string &resourceId, const std::string &dir,
        const std::shared_ptr<request::Request> &request, const std::string &name, const std::string &revision)
        : File(baseUrl, dir, request, name, revision), resourceId(resourceId){};
    void rm() override;
    bool pollChange(bool longPoll = false) override;
    bool supportsLongPoll() const override {
        return false;
    }
    std::string read() const override;
    void write(const std::string &content) override;

  private:
    const std::string resourceId;
};
} // namespace CloudSync::gdrive
