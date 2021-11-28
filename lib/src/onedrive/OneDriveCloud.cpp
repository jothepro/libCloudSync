#include "OneDriveCloud.hpp"

using namespace CloudSync;

void onedrive::OneDriveCloud::handleExceptions(const std::exception_ptr &e, const std::string &resourcePath) {
    try {
        std::rethrow_exception(e);
    } catch(request::Response::Conflict &e) {
        throw Resource::ResourceConflict(resourcePath);
    } catch (request::Response::NotFound &e) {
        throw Resource::NoSuchResource(resourcePath);
    } catch (request::Response::Forbidden &e) {
        throw Resource::PermissionDenied(resourcePath);
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

std::string onedrive::OneDriveCloud::get_user_display_name() const {
    std::string userDisplayName;
    try {
        const auto getResponse = this->request->GET("https://graph.microsoft.com/v1.0/me").json();
        userDisplayName = getResponse.at("displayName");
    } catch (...) {
        OneDriveCloud::handleExceptions(std::current_exception(), "");
    }
    return userDisplayName;
}

void onedrive::OneDriveCloud::logout() {
    // token revoking is not supported.
    this->request->resetAuth();
}
