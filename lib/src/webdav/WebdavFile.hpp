#include <utility>

#include "FileImpl.hpp"
#include "credentials/BasicCredentialsImpl.hpp"
#include "request/Request.hpp"

namespace CloudSync::webdav {
    class WebdavFile : public FileImpl {
    public:
        WebdavFile(
                const std::string &baseUrl, const std::string &dir,
                std::shared_ptr<credentials::BasicCredentialsImpl> credentials,
                const std::shared_ptr<request::Request> &request,
                const std::string &name, const std::string &revision)
                : FileImpl(baseUrl, dir, request, name, revision)
                , m_credentials(std::move(credentials)) {};

        void remove() override;

        bool poll_change() override;

        [[nodiscard]] std::string read_as_string() const override;

        void write_string(const std::string &content) override;

    private:
        static const std::string XML_QUERY;
        const std::shared_ptr<credentials::BasicCredentialsImpl> m_credentials;

        [[nodiscard]] std::string resource_path() const override;
    };
} // namespace CloudSync::webdav
