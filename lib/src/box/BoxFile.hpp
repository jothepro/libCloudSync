#include "FileImpl.hpp"
#include "request/Request.hpp"

namespace CloudSync::box {
class BoxFile : public FileImpl {
  public:
    BoxFile(
        const std::string &resourceId, const std::string &dir, const std::shared_ptr<request::Request> &request,
        const std::string &name, const std::string &revision)
        : FileImpl("", dir, request, name, revision), resourceId(resourceId){};
    void rm() override;
    bool pollChange(bool longPoll = false) override;
    bool supportsLongPoll() const override {
        return true;
    }
    std::string read() const override;
    void write(const std::string &content) override;

  private:
    const std::string resourceId;
};
} // namespace CloudSync::box
