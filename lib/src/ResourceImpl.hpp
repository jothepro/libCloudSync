#pragma once

#include "CloudSync/Permissions.hpp"
#include "CloudSync/Resource.hpp"
#include <chrono>
#include <string>

using namespace std::chrono;

namespace CloudSync {
class ResourceImpl : public Resource {
  public:
    ResourceImpl(const std::string &name, system_clock::time_point lastModified, Permissions permissions)
        : _name(name), _lastModified(lastModified), _permissions(permissions) {}
    std::string name() override {
        return _name;
    };
    system_clock::time_point lastModified() override {
        return _lastModified;
    };
    Permissions permissions() override {
        return _permissions;
    };

  private:
    std::string _name;
    system_clock::time_point _lastModified;
    Permissions _permissions;
};
} // namespace CloudSync
