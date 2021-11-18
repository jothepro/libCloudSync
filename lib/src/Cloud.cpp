#include "CloudSync/Cloud.hpp"
#include "CloudSync/Credentials.hpp"
#include "CloudSync/Exceptions.hpp"
#include "request/Request.hpp"
#include "box/BoxCloud.hpp"
#include <iostream>

using namespace CloudSync;

namespace CloudSync {

std::ostream &operator<<(std::ostream &output, const std::shared_ptr<Cloud>& cloud) {
    output << "[Cloud url: " << cloud->getBaseUrl() << "]" << std::endl;
    return output;
}

} // namespace CloudSync
