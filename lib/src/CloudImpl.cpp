#include "CloudImpl.hpp"
#include "request/Request.hpp"
#include <utility>

using namespace CloudSync;

CloudImpl::CloudImpl(std::string url, std::shared_ptr<request::Request> request)
        : m_base_url(std::move(url)), m_request(std::move(request)) {}

void CloudImpl::test_connection() const {
    this->root()->list_resources();
}

std::string CloudImpl::get_base_url() const {
    return m_base_url;
}
