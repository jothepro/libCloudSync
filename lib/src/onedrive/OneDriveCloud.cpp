#include "OneDriveCloud.hpp"
#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::onedrive;

void OneDriveCloud::handleExceptions(const std::exception_ptr &e, const std::string &resource_path) {
    try {
        std::rethrow_exception(e);
    } catch(request::Response::Conflict &e) {
        throw exceptions::resource::ResourceConflict(resource_path);
    } catch (request::Response::NotFound &e) {
        throw exceptions::resource::NoSuchResource(resource_path);
    } catch (request::Response::Forbidden &e) {
        throw exceptions::resource::PermissionDenied(resource_path);
    } catch (request::Response::Unauthorized &e) {
        throw exceptions::cloud::AuthorizationFailed();
    } catch (request::Response::PreconditionFailed &e) {
        try {
            json errorJson = json::parse(e.data());
            const std::string errorMessage = errorJson.at("error").at("message");
            if (errorMessage == "ETag does not match current item's value") {
                throw exceptions::resource::ResourceHasChanged(errorMessage);
            } else {
                throw exceptions::cloud::CommunicationError(errorMessage);
            }
        } catch (json::exception &e) {
            throw exceptions::cloud::InvalidResponse(e.what());
        }
    } catch (request::Response::ResponseException &e) {
        throw exceptions::cloud::CommunicationError(e.what());
    } catch (json::exception &e) {
        throw exceptions::cloud::InvalidResponse(e.what());
    } catch (request::Request::RequestException &e) {
        throw exceptions::cloud::CommunicationError(e.what());
    }
}

std::string OneDriveCloud::get_user_display_name() const {
    std::string user_display_name;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->GET("https://graph.microsoft.com/v1.0/me")
                ->token_auth(token)
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
}
