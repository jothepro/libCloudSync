#include "BoxCloud.hpp"

using namespace CloudSync;
using namespace CloudSync::request;
using P = Request::ParameterType;

void box::BoxCloud::handleExceptions(const std::exception_ptr &e, const std::string &resourcePath) {
    try {
        std::rethrow_exception(e);
    } catch (request::Response::NotFound &e) {
        throw Resource::NoSuchFileOrDirectory(resourcePath);
    } catch (request::Response::Forbidden &e) {
        throw Resource::PermissionDenied(resourcePath);
    } catch (request::Response::Unauthorized &e) {
        throw Cloud::AuthorizationFailed();
    } catch (request::Response::ResponseException &e) {
        throw Cloud::CommunicationError(e.what());
    } catch (request::Request::RequestException &e) {
        throw Cloud::CommunicationError(e.what());
    } catch (nlohmann::json::exception &e) {
        throw Cloud::InvalidResponse(e.what());
    }
}

std::string box::BoxCloud::getUserDisplayName() const {
    std::string userDisplayName;
    try {
        const auto getResponse = this->request->GET("https://api.box.com/2.0/users/me").json();
        userDisplayName = getResponse.at("name");
    } catch (...) {
        BoxCloud::handleExceptions(std::current_exception(), "");
    }
    return userDisplayName;
}

void box::BoxCloud::logout() {
    // logout without providing a client-Id seems to be not supported:
    // https://developer.box.com/guides/authentication/tokens/revoke/
    this->request->resetAuth();
}
