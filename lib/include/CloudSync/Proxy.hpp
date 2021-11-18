#pragma once

#include <memory>
#include <string>
#include <utility>

namespace CloudSync {
namespace request {
class Request;
}
class Proxy {
    friend class CloudImpl;

  public:
    const static Proxy NOPROXY;
    explicit Proxy(std::string url, std::string username = "", std::string password = "")
        : url(std::move(url))
        , username(std::move(username))
        , password(std::move(password)){};

  protected:
    virtual void apply(const std::shared_ptr<request::Request> &request) const;

  private:
    const std::string url;
    const std::string username;
    const std::string password;
};
} // namespace CloudSync
