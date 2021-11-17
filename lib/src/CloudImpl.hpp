#pragma once

#include "CloudSync/Cloud.hpp"

namespace CloudSync {
class CloudImpl : public Cloud, public std::enable_shared_from_this<CloudImpl> {
  public:
    void ping() const override;
    std::shared_ptr<Cloud> login(const Credentials &credentials) override;
    std::string getCurrentRefreshToken() const override;
    std::shared_ptr<Cloud> proxy(const Proxy &proxy) override;
    std::string getBaseUrl() const override;
    virtual ~CloudImpl() = default;

  protected:
    CloudImpl(const std::string &url, const std::shared_ptr<request::Request> &request);
    std::shared_ptr<request::Request> request;
    std::string baseUrl;
};
} // namespace CloudSync
