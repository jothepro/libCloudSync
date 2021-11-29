#include "CloudSync/OAuth2Credentials.hpp"
#include "request/Request.hpp"
#include <nlohmann/json.hpp>

using namespace std::chrono;
using namespace std::literals::chrono_literals;
using json = nlohmann::json;
using namespace CloudSync::request;

namespace CloudSync {

    void OAuth2Credentials::apply(const std::shared_ptr<request::Request> &request) const {
        request->set_oauth2(accessToken, refreshToken, expires);
    }
} // namespace CloudSync
