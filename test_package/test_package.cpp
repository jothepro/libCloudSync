#include "CloudSync/CloudFactory.hpp"
#include <memory>

using namespace CloudSync;

int main() {
    auto cloud = CloudFactory().dropbox();
}
