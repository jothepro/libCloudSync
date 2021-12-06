#include "CloudSync/CloudFactory.hpp"
#include <memory>

using namespace CloudSync;

int main() {
    auto credentials = OAuth2Credentials::from_access_token("abc");
    auto cloud = CloudFactory().create_dropbox(credentials);
}
