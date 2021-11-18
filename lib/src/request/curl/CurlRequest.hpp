#include "request/Request.hpp"
#include <curl/curl.h>
#include <unordered_map>

namespace CloudSync::request::curl {
    class CurlRequest : public Request {
    public:
        CurlRequest();

        ~CurlRequest() override;

        Response request(
                const std::string &verb, const std::string &url,
                const std::unordered_map<ParameterType, const std::unordered_map<std::string, std::string>> &parameters,
                const std::string &body) override;

        static const std::string WHITESPACE;

    private:
        CURL *curl;

        std::string urlEncodeParams(const std::unordered_map<std::string, std::string> &params) const;
    };
} // namespace CloudSync::request::curl
