#include "BasicCredentialsImpl.hpp"

using namespace CloudSync;
using namespace CloudSync::credentials;

std::string BasicCredentialsImpl::username() {
    return m_username;
}

std::string BasicCredentialsImpl::password() {
    return m_password;
}
