#include "CloudSync/Directory.hpp"

namespace CloudSync {
Directory::Directory(
    const std::string &baseUrl, const std::string &dir, const std::shared_ptr<request::Request> &request,
    const std::string &name = "")
    : Resource(baseUrl, dir, request, name) {}

std::string Directory::pwd() const {
    return this->path;
}
} // namespace CloudSync
