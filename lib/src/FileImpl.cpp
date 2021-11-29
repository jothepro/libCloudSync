#include "FileImpl.hpp"

using namespace CloudSync;

std::string FileImpl::name() const {
    return m_name;
}

std::string FileImpl::path() const {
    return m_path;
}

std::string CloudSync::FileImpl::revision() const {
    return m_revision;
}

bool FileImpl::is_file() const {
    return true;
}
