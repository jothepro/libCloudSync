#include "GDriveCloud.hpp"

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::gdrive;

void GDriveCloud::handleExceptions(const std::exception_ptr &e, const std::string &resourcePath) {
    try {
        std::rethrow_exception(e);
    } catch (request::Response::NotFound &e) {
        try {
            const auto errorMessage = json::parse(e.data());
            if (errorMessage["error"]["errors"][0]["reason"] == "notFound") {
                throw Resource::NoSuchResource(resourcePath);
            } else {
                throw Cloud::CommunicationError("unknown error response: " + e.data());
            }
        } catch (json::exception &e) {
            throw Cloud::InvalidResponse(e.what());
        }
    } catch (request::Response::Unauthorized &e) {
        throw Cloud::AuthorizationFailed();
    } catch (request::Response::PreconditionFailed &e) {
        throw Resource::ResourceHasChanged(resourcePath);
    } catch (request::Response::Forbidden &) {
        throw Resource::PermissionDenied(resourcePath);
    } catch (json::exception &e) {
        throw Cloud::InvalidResponse(e.what());
    } catch (request::Request::RequestException &e) {
        throw Cloud::CommunicationError(e.what());
    }
}

std::string GDriveCloud::get_user_display_name() const {
    std::string user_display_name;
    try {
        const auto response_json = m_request->GET("https://www.googleapis.com/userinfo/v2/me")
                ->accept(Request::MIMETYPE_JSON)
                ->send().json();
        user_display_name = response_json.at("name");
    } catch (...) {
        GDriveCloud::handleExceptions(std::current_exception(), "");
    }
    return user_display_name;
}

void GDriveCloud::logout() {
    try {
        m_request->POST("https://oauth2.googleapis.com/revoke")
                ->mime_postfield("token", m_request->get_current_access_token())
                ->send();
        m_request->reset_auth();
    } catch (...) {
        GDriveCloud::handleExceptions(std::current_exception(), "");
    }
}
