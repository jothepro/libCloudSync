#include "OAuthFileImpl.hpp"
#include "request/Request.hpp"

namespace CloudSync::dropbox {
    class DropboxFile : public OAuthFileImpl {
    public:
        DropboxFile(
                const std::string &dir,
                const std::shared_ptr<credentials::OAuth2CredentialsImpl>& credentials,
                const std::shared_ptr<request::Request> &request, const std::string &name,
                const std::string &revision)
                : OAuthFileImpl("", dir, credentials, request, name, revision) {};

        void remove() override;

        bool poll_change() override;

        [[nodiscard]] std::string read() const override;

        [[nodiscard]] std::vector<std::uint8_t> read_binary() const override;

        void write(const std::string& content) override;

        void write_binary(const std::vector<std::uint8_t> & content) override;

    private:
        std::shared_ptr<request::Request> prepare_read_request() const;
        std::shared_ptr<request::Request> prepare_write_request() const;
    };
} // namespace CloudSync::dropbox
