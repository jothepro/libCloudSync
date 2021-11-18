#pragma once

#include "Cloud.hpp"
#include "Credentials.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <utility>

using namespace std::chrono_literals;

namespace CloudSync {
namespace request {
class Response;
}
class OAuth2Credentials : public Credentials {
    friend class Cloud;

  public:
    explicit OAuth2Credentials(
        std::string accessToken,
        std::string refreshToken = "",
        std::chrono::seconds expiresIn = 0s)
        : accessToken(std::move(accessToken))
        , refreshToken(std::move(refreshToken))
        , expires(expiresIn != 0s
            ? std::chrono::system_clock::now() + expiresIn
            : std::chrono::system_clock::time_point(0s)){};

  protected:
    void apply(const std::shared_ptr<request::Request> &request) const override;

  private:
    const std::string accessToken;
    const std::string refreshToken;
    const std::chrono::system_clock::time_point expires;
};
} // namespace CloudSync
