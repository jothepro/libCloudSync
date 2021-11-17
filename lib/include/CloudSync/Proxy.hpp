#pragma once

#include <memory>
#include <string>
#include <utility>

class CloudImpl;

namespace CloudSync {
namespace request {
class Request;
}
class Proxy {
    friend class CloudImpl;

  public:
    const static Proxy NOPROXY;
    Proxy(const std::string &url, const std::string &username = "", const std::string &password = "")
        : url(url), username(username), password(password){};

  protected:
    virtual void apply(const std::shared_ptr<request::Request> &request) const;

  private:
    const std::string url;
    const std::string username;
    const std::string password;
};
} // namespace CloudSync
