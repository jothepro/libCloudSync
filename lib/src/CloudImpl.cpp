#include "CloudImpl.hpp"
#include "CloudSync/Credentials.hpp"
#include "request/Request.hpp"
#include <utility>

using namespace CloudSync;

CloudImpl::CloudImpl(std::string url, std::shared_ptr<request::Request> request)
        : m_base_url(std::move(url)), m_request(std::move(request)) {}

std::shared_ptr<Cloud> CloudImpl::login(const Credentials &credentials) {
    m_request->set_token_request_url(this->getTokenUrl());
    credentials.apply(m_request);
    return this->shared_from_this();
}

std::string CloudImpl::getCurrentRefreshToken() const {
    return m_request->get_current_refresh_token();
}

void CloudImpl::ping() const {
    this->root()->list_resources();
}

std::string CloudImpl::getBaseUrl() const {
    return m_base_url;
}
