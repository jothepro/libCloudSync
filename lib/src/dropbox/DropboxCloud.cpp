#include "DropboxCloud.hpp"
#include "DropboxExceptionTranslator.hpp"

using namespace CloudSync;
using namespace CloudSync::dropbox;

void DropboxCloud::logout() {
    try {
        m_request->POST("https://api.dropboxapi.com/2/auth/token/revoke")
                ->token_auth(m_credentials->get_current_access_token())
                ->request();
    } catch (...) {
        DropboxExceptionTranslator::translate();
    }
}

std::string DropboxCloud::get_user_display_name() const {
    std::string user_display_name;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->POST("https://api.dropboxapi.com/2/users/get_current_account")
                ->token_auth(token)
                ->accept(Request::MIMETYPE_JSON)
                ->request().json();
        user_display_name = response_json.at("name").at("display_name");

    } catch (...) {
        DropboxExceptionTranslator::translate();
    }
    return user_display_name;
}
