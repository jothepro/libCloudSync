#include "FileImpl.hpp"
#include "request/Request.hpp"

namespace CloudSync::box {
    class BoxFile : public FileImpl {
    public:
        BoxFile(
                const std::string &resourceId, const std::string &dir, const std::shared_ptr<request::Request> &request,
                const std::string &name, const std::string &revision)
                : FileImpl("", dir, request, name, revision), resourceId(resourceId) {};

        void remove() override;

        bool poll_change(bool longPoll = false) override;

        bool supports_long_poll() const override {
            return true;
        }

        std::string read_as_string() const override;

        void write_string(const std::string &content) override;

    private:
        const std::string resourceId;
    };
} // namespace CloudSync::box
