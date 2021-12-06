#include "GDriveCloud.hpp"
#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"

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
                throw exceptions::resource::NoSuchResource(resourcePath);
            } else {
                throw exceptions::cloud::CommunicationError("unknown error response: " + e.data());
            }
        } catch (json::exception &e) {
            throw exceptions::cloud::InvalidResponse(e.what());
        }
    } catch (request::Response::Unauthorized &e) {
        throw exceptions::cloud::AuthorizationFailed();
    } catch (request::Response::PreconditionFailed &e) {
        throw exceptions::resource::ResourceHasChanged(resourcePath);
    } catch (request::Response::Forbidden &) {
        throw exceptions::resource::PermissionDenied(resourcePath);
    } catch (json::exception &e) {
        throw exceptions::cloud::InvalidResponse(e.what());
    } catch (request::Request::RequestException &e) {
        throw exceptions::cloud::CommunicationError(e.what());
    }
}

std::string GDriveCloud::get_user_display_name() const {
    std::string user_display_name;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->GET("https://www.googleapis.com/userinfo/v2/me")
                ->token_auth(token)
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
        const auto token = m_credentials->get_current_access_token();
        m_request->POST("https://oauth2.googleapis.com/revoke")
                ->token_auth(token)
                ->mime_postfield("token", m_credentials->get_current_access_token())
                ->send();
    } catch (...) {
        GDriveCloud::handleExceptions(std::current_exception(), "");
    }
}
