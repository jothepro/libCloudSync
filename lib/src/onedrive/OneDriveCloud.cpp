#include "OneDriveCloud.hpp"

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::onedrive;

void OneDriveCloud::handleExceptions(const std::exception_ptr &e, const std::string &resource_path) {
    try {
        std::rethrow_exception(e);
    } catch(request::Response::Conflict &e) {
        throw Resource::ResourceConflict(resource_path);
    } catch (request::Response::NotFound &e) {
        throw Resource::NoSuchResource(resource_path);
    } catch (request::Response::Forbidden &e) {
        throw Resource::PermissionDenied(resource_path);
    } catch (request::Response::Unauthorized &e) {
        throw Cloud::AuthorizationFailed();
    } catch (request::Response::PreconditionFailed &e) {
        try {
            json errorJson = json::parse(e.data());
            const std::string errorMessage = errorJson.at("error").at("message");
            if (errorMessage == "ETag does not match current item's value") {
                throw Resource::ResourceHasChanged(errorMessage);
            } else {
                throw Cloud::CommunicationError(errorMessage);
            }
        } catch (json::exception &e) {
            throw Cloud::InvalidResponse(e.what());
        }
    } catch (request::Response::ResponseException &e) {
        throw Cloud::CommunicationError(e.what());
    } catch (json::exception &e) {
        throw Cloud::InvalidResponse(e.what());
    } catch (request::Request::RequestException &e) {
        throw Cloud::CommunicationError(e.what());
    }
}

std::string OneDriveCloud::get_user_display_name() const {
    std::string user_display_name;
    try {
        const auto response_json = m_request->GET("https://graph.microsoft.com/v1.0/me")
                ->accept(Request::MIMETYPE_JSON)
                ->send().json();
        user_display_name = response_json.at("displayName");
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), "");
    }
    return user_display_name;
}

void OneDriveCloud::logout() {
    // token revoking is not supported.
    m_request->reset_auth();
}
