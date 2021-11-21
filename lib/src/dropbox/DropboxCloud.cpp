#include "DropboxCloud.hpp"

using namespace CloudSync;

void dropbox::DropboxCloud::logout() {
    try {
        this->request->POST("https://api.dropboxapi.com/2/auth/token/revoke");
        this->request->resetAuth();
    } catch (...) {
        DropboxCloud::handleExceptions(std::current_exception(), "");
    }
}

std::string dropbox::DropboxCloud::getUserDisplayName() const {
    std::string userDisplayName;
    try {
        const auto getResponse =
                this->request->POST("https://api.dropboxapi.com/2/users/get_current_account").json();
        userDisplayName = getResponse.at("name").at("display_name");

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
                throw Resource::NoSuchFileOrDirectory(resourcePath);
            } else {
                throw Cloud::CommunicationError(e.what());
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
