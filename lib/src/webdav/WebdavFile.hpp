#include "CloudSync/File.hpp"
#include "request/Request.hpp"

namespace CloudSync::webdav {
class WebdavFile : public File {
  public:
    WebdavFile(
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

  private:
    static std::string xmlQuery;
};
} // namespace CloudSync::webdav
