#pragma once

#include "Cloud.hpp"
#include "Credentials.hpp"
#include <string>
#include <utility>

namespace CloudSync {
    class UsernamePasswordCredentials : public Credentials {
    public:
        UsernamePasswordCredentials(std::string username, std::string password)
                : username(std::move(username)), password(std::move(password)) {};

    protected:
        void apply(const std::shared_ptr<request::Request> &request) const override;

    private:
        std::string username;
        std::string password;
    };
} // namespace CloudSync
