#include "NextcloudCloud.hpp"

using namespace CloudSync;
using namespace CloudSync::nextcloud;

std::string NextcloudCloud::get_user_display_name() const {
    std::string user_display_name;
    try {
        const auto response_xml = m_request->GET(m_base_url + "/ocs/v1.php/cloud/user")
                ->basic_auth(m_credentials->username(), m_credentials->password())
                ->header("OCS-APIRequest", "true")
                ->accept(Request::MIMETYPE_XML)
                ->send().xml();
        user_display_name = response_xml->select_node("/ocs/data/display-name").node().child_value();
    } catch (...) {
        NextcloudCloud::handleExceptions(std::current_exception(), "");
    }
    return user_display_name;
}

void NextcloudCloud::logout() {
    try {
        m_request->DELETE(m_base_url + "/ocs/v2.php/core/apppassword")
                ->basic_auth(m_credentials->username(), m_credentials->password())
                ->header("OCS-APIRequest", "true")
                ->accept(Request::MIMETYPE_XML)
                ->send();
    } catch (...) {
        NextcloudCloud::handleExceptions(std::current_exception(), "");
    }
}
