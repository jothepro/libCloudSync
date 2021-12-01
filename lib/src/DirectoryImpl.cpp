#include "DirectoryImpl.hpp"
#include <utility>

using namespace CloudSync;

DirectoryImpl::DirectoryImpl(
        std::string baseUrl,
        std::string dir,
        std::shared_ptr<request::Request> request,
        std::string name = "")
        : m_base_url(std::move(baseUrl))
        , m_path(std::move(dir))
        , m_request(std::move(request))
        , m_name(std::move(name)) {}

std::string DirectoryImpl::name() const {
    return m_name;
}

std::string DirectoryImpl::path() const {
    return m_path;
}

bool DirectoryImpl::is_file() const {
    return false;
}
