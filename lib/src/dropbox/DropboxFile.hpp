#include "FileImpl.hpp"
#include "request/Request.hpp"

namespace CloudSync::dropbox {
    class DropboxFile : public FileImpl {
    public:
        DropboxFile(
                const std::string &dir, const std::shared_ptr<request::Request> &request, const std::string &name,
                const std::string &revision)
                : FileImpl("", dir, request, name, revision) {};

        void remove() override;

        bool poll_change(bool longPoll = false) override;

        [[nodiscard]] bool supports_long_poll() const override {
            return true;
        }

        [[nodiscard]] std::string read_as_string() const override;

        void write_string(const std::string &content) override;
    };
} // namespace CloudSync::dropbox
