#include "DirectoryImpl.hpp"
#include <utility>

using namespace CloudSync;

std::string DirectoryImpl::name() const {
    return m_name;
}

std::filesystem::path DirectoryImpl::path() const {
    return m_path;
}

bool DirectoryImpl::is_file() const {
    return false;
}

std::filesystem::path DirectoryImpl::append_path(const std::filesystem::path &child_path) const {
    std::string full_path = (m_path / child_path).lexically_normal().generic_string();
    return {remove_trailing_slashes(full_path)};
}

std::string DirectoryImpl::remove_trailing_slashes(const std::string& input) {
    std::string output = input;
    while (output.size() > 2 && output.back() == '/') {
        output.erase(output.size() - 1);
    }
    return output;
}
