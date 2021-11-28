#include "CloudImpl.hpp"

#include <utility>
#include "CloudSync/Credentials.hpp"
#include "request/Request.hpp"
#include "request/curl/CurlRequest.hpp"

using namespace CloudSync::request::curl;


using C = CloudSync::request::Request::ConfigurationOption;

namespace CloudSync {

    CloudImpl::CloudImpl(std::string url, std::shared_ptr<request::Request> request)
            : baseUrl(std::move(url)), request(std::move(request)) {}

    std::shared_ptr<Cloud> CloudImpl::login(const Credentials &credentials) {
        this->request->setTokenRequestUrl(this->getTokenUrl());
        credentials.apply(this->request);
        return this->shared_from_this();
    }

    std::string CloudImpl::getCurrentRefreshToken() const {
        return this->request->getCurrentRefreshToken();
    }

    void CloudImpl::ping() const {
        this->root()->list_resources();
    }

    std::string CloudImpl::getBaseUrl() const {
        return this->baseUrl;
    }

} // namespace CloudSync
