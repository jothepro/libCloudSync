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

        [[nodiscard]] std::string read_as_string() const override;

        void write_string(const std::string &content) override;
    private:
        [[nodiscard]] std::string resource_path() const override;
    };
} // namespace CloudSync::dropbox
