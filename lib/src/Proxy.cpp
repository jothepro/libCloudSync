#include "CloudSync/Proxy.hpp"
#include "request/Request.hpp"

namespace CloudSync {

    const Proxy Proxy::NOPROXY = Proxy("");

    void Proxy::apply(const std::shared_ptr<request::Request> &request) const {
        request->setProxy(this->url, this->username, this->password);
    }

} // namespace CloudSync
