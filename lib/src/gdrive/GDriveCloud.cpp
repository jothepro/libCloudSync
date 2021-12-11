#include "GDriveCloud.hpp"
#include "GDriveExceptionTranslator.hpp"

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::gdrive;

std::string GDriveCloud::get_user_display_name() const {
    std::string user_display_name;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->GET("https://www.googleapis.com/userinfo/v2/me")
                ->token_auth(token)
                ->accept(Request::MIMETYPE_JSON)
                ->request().json();
        user_display_name = response_json.at("name");
    } catch (...) {
        GDriveExceptionTranslator::translate();
    }
    return user_display_name;
}

void GDriveCloud::logout() {
    try {
        const auto token = m_credentials->get_current_access_token();
        m_request->POST("https://oauth2.googleapis.com/revoke")
                ->token_auth(token)
                ->mime_postfield("token", m_credentials->get_current_access_token())
                ->request();
    } catch (...) {
        GDriveExceptionTranslator::translate();
    }
}
