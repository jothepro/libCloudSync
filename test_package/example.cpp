#include "CloudSync/CloudFactory.hpp"
#include <memory>

using namespace CloudSync;

int main() {
    auto factory = std::make_shared<CloudFactory>();
    factory->dropbox();
}
