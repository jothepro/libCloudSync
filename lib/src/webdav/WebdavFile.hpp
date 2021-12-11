#include <utility>

#include "FileImpl.hpp"
#include "credentials/BasicCredentialsImpl.hpp"
#include "request/Request.hpp"

namespace CloudSync::webdav {
    class WebdavFile : public FileImpl {
    public:
        WebdavFile(
                const std::string &baseUrl, const std::filesystem::path &dir,
                std::shared_ptr<credentials::BasicCredentialsImpl> credentials,
                const std::shared_ptr<request::Request> &request,
                const std::string &name, const std::string &revision)
                : FileImpl(baseUrl, dir, request, name, revision)
                , m_credentials(std::move(credentials))
                , m_resource_path(m_base_url + m_path.generic_string()){
        };

        void remove() override;

        bool poll_change() override;

        [[nodiscard]] std::string read() const override;
        [[nodiscard]] std::vector<std::uint8_t> read_binary() const override;

        void write(const std::string& content) override;
        void write_binary(const std::vector<std::uint8_t>& content) override;

    private:
        static const std::string XML_QUERY;
        const std::shared_ptr<credentials::BasicCredentialsImpl> m_credentials;

        const std::string m_resource_path;
    };
} // namespace CloudSync::webdav
