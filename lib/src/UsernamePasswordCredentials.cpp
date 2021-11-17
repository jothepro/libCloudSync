#include "CloudSync/UsernamePasswordCredentials.hpp"
#include "request/Request.hpp"

using namespace CloudSync::request;

namespace CloudSync {
void UsernamePasswordCredentials::apply(const std::shared_ptr<request::Request> &request) const {
    request->setBasicAuth(username, password);
}
} // namespace CloudSync
