#include "DropboxCloud.hpp"
#include "CloudSync/exceptions/Exception.hpp"
#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"

using namespace CloudSync;
using namespace CloudSync::dropbox;

void DropboxCloud::logout() {
    try {
        m_request->POST("https://api.dropboxapi.com/2/auth/token/revoke")
                ->token_auth(m_credentials->get_current_access_token())
                ->send();
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), "");
    }
}

std::string DropboxCloud::get_user_display_name() const {
    std::string userDisplayName;
    try {
        const auto token = m_credentials->get_current_access_token();
        const auto response_json = m_request->POST("https://api.dropboxapi.com/2/users/get_current_account")
                ->token_auth(token)
                ->accept(Request::MIMETYPE_JSON)
                ->send().json();
        userDisplayName = response_json.at("name").at("display_name");

    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), "");
    }
    return userDisplayName;
}

void DropboxCloud::handleExceptions(const std::exception_ptr &e, const std::string &resourcePath) {
    try {
        std::rethrow_exception(e);
    } catch (request::Response::Conflict &e) {
        try {
            json error_json = json::parse(e.data());
            if ((error_json["error"][".tag"] == "path" && error_json["error"]["path"][".tag"] == "not_found") ||
                (error_json["error"][".tag"] == "path_lookup" &&
                 error_json["error"]["path_lookup"][".tag"] == "not_found")) {
                throw exceptions::resource::NoSuchResource(resourcePath);
            } else {
                throw exceptions::resource::ResourceConflict(e.what());
            }
        } catch (json::exception &e) {
            throw exceptions::cloud::InvalidResponse(e.what());
        }
    } catch (request::Response::Unauthorized &e) {
        throw exceptions::cloud::AuthorizationFailed();
    } catch (request::Response::ResponseException &e) {
        throw exceptions::cloud::CommunicationError(e.what());
    } catch (Response::ParseError &e) {
        throw exceptions::cloud::InvalidResponse(e.what());
    } catch (request::Request::RequestException &e) {
        throw exceptions::cloud::CommunicationError(e.what());
    } catch (json::exception &e) {
        throw exceptions::cloud::InvalidResponse(e.what());
    }
}
