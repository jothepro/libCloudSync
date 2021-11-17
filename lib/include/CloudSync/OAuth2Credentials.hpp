#pragma once

#include "Cloud.hpp"
#include "Credentials.hpp"
#include <chrono>
#include <memory>
#include <string>

namespace CloudSync {
namespace request {
class Response;
}
class OAuth2Credentials : public Credentials {
    friend class Cloud;

  public:
    OAuth2Credentials(
        const std::string &accessToken, const std::string &refreshToken = "",
        std::chrono::seconds expiresIn = std::chrono::seconds(0))
        : accessToken(accessToken), refreshToken(refreshToken),
          expires(
              expiresIn != std::chrono::seconds(0) ? std::chrono::system_clock::now() + expiresIn
                                                   : std::chrono::system_clock::time_point(std::chrono::seconds(0))){};

  protected:
    void apply(const std::shared_ptr<request::Request> &request) const override;

  private:
    const std::string accessToken;
    const std::string refreshToken;
    const std::chrono::system_clock::time_point expires;
};
} // namespace CloudSync
