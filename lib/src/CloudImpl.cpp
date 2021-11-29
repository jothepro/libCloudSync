#include "CloudImpl.hpp"

#include <utility>
#include "CloudSync/Credentials.hpp"
#include "request/Request.hpp"

namespace CloudSync {

    CloudImpl::CloudImpl(std::string url, std::shared_ptr<request::Request> request)
            : baseUrl(std::move(url)), request(std::move(request)) {}

    std::shared_ptr<Cloud> CloudImpl::login(const Credentials &credentials) {
        this->request->set_token_request_url(this->getTokenUrl());
        credentials.apply(this->request);
        return this->shared_from_this();
    }

    std::string CloudImpl::getCurrentRefreshToken() const {
        return this->request->get_current_refresh_token();
    }

    void CloudImpl::ping() const {
        this->root()->list_resources();
    }

    std::string CloudImpl::getBaseUrl() const {
        return this->baseUrl;
    }

} // namespace CloudSync
