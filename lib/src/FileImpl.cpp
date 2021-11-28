#include "FileImpl.hpp"

using namespace CloudSync;

std::string FileImpl::name() const {
    return _name;
}

std::string FileImpl::path() const {
    return _path;
}

std::string CloudSync::FileImpl::revision() const {
    return _revision;
}

bool FileImpl::is_file() const {
    return true;
}
