#include "FileImpl.hpp"
#include "request/Request.hpp"

namespace CloudSync::dropbox {
    class DropboxFile : public FileImpl {
    public:
        DropboxFile(
                const std::string &dir, const std::shared_ptr<request::Request> &request, const std::string &name,
                const std::string &revision)
                : FileImpl("", dir, request, name, revision) {};

        void rm() override;

        bool pollChange(bool longPoll = false) override;

        [[nodiscard]] bool supportsLongPoll() const override {
            return true;
        }

        [[nodiscard]] std::string read() const override;

        void write(const std::string &content) override;
    };
} // namespace CloudSync::dropbox
