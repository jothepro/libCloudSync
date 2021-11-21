#include "GDriveCloud.hpp"

using namespace CloudSync;
using namespace CloudSync::request;
using P = Request::ParameterType;

void gdrive::GDriveCloud::handleExceptions(const std::exception_ptr &e, const std::string &resourcePath) {
    try {
        std::rethrow_exception(e);
    } catch (request::Response::NotFound &e) {
        try {
            const auto errorMessage = json::parse(e.data());
            if (errorMessage["error"]["errors"][0]["reason"] == "notFound") {
                throw Resource::NoSuchFileOrDirectory(resourcePath);
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

std::string gdrive::GDriveCloud::getUserDisplayName() const {
    std::string userDisplayName;
    try {
        const auto getResponse = this->request->GET("https://www.googleapis.com/userinfo/v2/me").json();
        userDisplayName = getResponse.at("name");
    } catch (...) {
        GDriveCloud::handleExceptions(std::current_exception(), "");
    }
    return userDisplayName;
}

void gdrive::GDriveCloud::logout() {
    try {
        this->request->POST(
            "https://oauth2.googleapis.com/revoke",
            {
                {
                    P::MIME_POSTFIELDS, {
                            {"token", this->request->getCurrentAccessToken()}
                    }
                }
            }
        );
        this->request->resetAuth();
    } catch (...) {
        GDriveCloud::handleExceptions(std::current_exception(), "");
    }
}
