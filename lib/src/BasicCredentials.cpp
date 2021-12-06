#include "CloudSync/BasicCredentials.hpp"
#include "credentials/BasicCredentialsImpl.hpp"

using namespace CloudSync;

std::shared_ptr<BasicCredentials>
BasicCredentials::from_username_password(const std::string &username, const std::string &password) {
    return std::make_shared<credentials::BasicCredentialsImpl>(username, password);
}
