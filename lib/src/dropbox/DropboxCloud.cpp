#include "DropboxCloud.hpp"

using namespace CloudSync;

void dropbox::DropboxCloud::logout() {
    try {
        this->request->POST("https://api.dropboxapi.com/2/auth/token/revoke");
        this->request->reset_auth();
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), "");
    }
}

std::string dropbox::DropboxCloud::get_user_display_name() const {
    std::string userDisplayName;
    try {
        const auto response_json = this->request->POST("https://api.dropboxapi.com/2/users/get_current_account")
                ->accept(Request::MIMETYPE_JSON)
                ->send().json();
        userDisplayName = response_json.at("name").at("display_name");

    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), "");
    }
    return userDisplayName;
}

void dropbox::DropboxCloud::handleExceptions(const std::exception_ptr &e, const std::string &resourcePath) {
    try {
        std::rethrow_exception(e);
    } catch (request::Response::Conflict &e) {
        try {
            json errorJson = json::parse(e.data());
            if ((errorJson["error"][".tag"] == "path" && errorJson["error"]["path"][".tag"] == "not_found") ||
                (errorJson["error"][".tag"] == "path_lookup" &&
                 errorJson["error"]["path_lookup"][".tag"] == "not_found")) {
                throw Resource::NoSuchResource(resourcePath);
            } else {
                throw Resource::ResourceConflict(e.what());
            }
        } catch (json::exception &e) {
            throw Cloud::InvalidResponse(e.what());
        }
    } catch (request::Response::Unauthorized &e) {
        throw Cloud::AuthorizationFailed();
    } catch (request::Response::ResponseException &e) {
        throw Cloud::CommunicationError(e.what());
    } catch (Response::ParseError &e) {
        throw Cloud::InvalidResponse(e.what());
    } catch (request::Request::RequestException &e) {
        throw Cloud::CommunicationError(e.what());
    } catch (json::exception &e) {
        throw Cloud::InvalidResponse(e.what());
    }
}
