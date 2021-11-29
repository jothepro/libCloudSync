#include "NextcloudCloud.hpp"

using namespace CloudSync;

std::string nextcloud::NextcloudCloud::get_user_display_name() const {
    std::string user_display_name;
    try {
        const auto response_xml = this->request->GET(this->baseUrl + "/ocs/v1.php/cloud/user")
                ->header("OCS-APIRequest", "true")
                ->accept(Request::MIMETYPE_XML)
                ->send().xml();
        user_display_name = response_xml->select_node("/ocs/data/display-name").node().child_value();
    } catch (...) {
        NextcloudCloud::handleExceptions(std::current_exception(), "");
    }
    return user_display_name;
}

void nextcloud::NextcloudCloud::logout() {
    try {
        this->request->DELETE(this->baseUrl + "/ocs/v2.php/core/apppassword")
                ->header("OCS-APIRequest", "true")
                ->accept(Request::MIMETYPE_XML)
                ->send();
        this->request->reset_auth();
    } catch (...) {
        NextcloudCloud::handleExceptions(std::current_exception(), "");
    }
}
