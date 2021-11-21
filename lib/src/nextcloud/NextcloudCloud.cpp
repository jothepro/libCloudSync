#include "NextcloudCloud.hpp"

using namespace CloudSync;

std::string nextcloud::NextcloudCloud::getUserDisplayName() const {
    std::string userDisplayName;
    try {
        const auto responseXml = this->request->GET(
            this->baseUrl + "/ocs/v1.php/cloud/user",
            {
                {
                    P::HEADERS, {
                        {"OCS-APIRequest", "true"},
                        {"Accept", Request::MIMETYPE_XML}
                    }
                }
            }
        ).xml();
        userDisplayName = responseXml->select_node("/ocs/data/display-name").node().child_value();
    } catch (...) {
        NextcloudCloud::handleExceptions(std::current_exception(), "");
    }
    return userDisplayName;
}

void nextcloud::NextcloudCloud::logout() {
    try {
        this->request->DELETE(
            this->baseUrl + "/ocs/v2.php/core/apppassword",
            {
                {
                    P::HEADERS, {
                        {"OCS-APIRequest", "true"},
                        {"Accept", Request::MIMETYPE_XML}
                    }
                }
            }
        );
        this->request->resetAuth();
    } catch (...) {
        NextcloudCloud::handleExceptions(std::current_exception(), "");
    }
}
