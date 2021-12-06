#pragma once

#include <utility>

#include "CloudSync/BasicCredentials.hpp"

namespace CloudSync::credentials {
    class BasicCredentialsImpl : public BasicCredentials {
    public:
        BasicCredentialsImpl(std::string  username, std::string  password)
        : m_username(std::move(username)), m_password(std::move(password)) {};

        virtual std::string username();
        virtual std::string password();
    private:
        std::string m_username;
        std::string m_password;
    };
}