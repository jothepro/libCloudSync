#include "OneDriveCloud.hpp"
#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "OneDriveExceptionTranslator.hpp"

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::onedrive;

std::string OneDriveCloud::get_user_display_name() const {
    std::string user_display_name;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->GET("https://graph.microsoft.com/v1.0/me")
                ->token_auth(token)
                ->accept(Request::MIMETYPE_JSON)
                ->request().json();
        user_display_name = response_json.at("displayName");
    } catch (...) {
        OneDriveExceptionTranslator::translate();
    }
    return user_display_name;
}

void OneDriveCloud::logout() {
    // token revoking is not supported.
}
