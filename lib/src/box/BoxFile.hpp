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

        bool poll_change() override;

        std::string read_as_string() const override;

        void write_string(const std::string &content) override;

    private:
        const std::string resourceId;

        [[nodiscard]] std::string resource_path() const override;
    };
} // namespace CloudSync::box
