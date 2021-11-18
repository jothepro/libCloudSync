#include "DirectoryImpl.hpp"

#include <utility>

namespace CloudSync {

    DirectoryImpl::DirectoryImpl(
            std::string baseUrl,
            std::string dir,
            std::shared_ptr<request::Request> request,
            std::string name = "")
            : _baseUrl(std::move(baseUrl))
            , _path(std::move(dir))
            , request(std::move(request))
            , _name(std::move(name)) {}

    std::string DirectoryImpl::name() const {
        return _name;
    }

    std::string DirectoryImpl::path() const {
        return _path;
    }

    std::string DirectoryImpl::pwd() const {
        return this->path();
    }

    bool DirectoryImpl::isFile() const {
        return false;
    }
} // namespace CloudSync
