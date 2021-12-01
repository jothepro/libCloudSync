#include "BoxCloud.hpp"

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::box;

void BoxCloud::handleExceptions(const std::exception_ptr &e, const std::string &resourcePath) {
    try {
        std::rethrow_exception(e);
    } catch (request::Response::NotFound &e) {
        throw Resource::NoSuchResource(resourcePath);
    } catch (request::Response::Forbidden &e) {
        throw Resource::PermissionDenied(resourcePath);
    } catch (request::Response::Conflict &e) {
        throw Resource::ResourceConflict(resourcePath);
    } catch (request::Response::PreconditionFailed &e) {
        throw Resource::ResourceHasChanged(resourcePath);
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

std::string BoxCloud::get_user_display_name() const {
    std::string user_display_name;
    try {
        const auto response_json = m_request->GET("https://api.box.com/2.0/users/me")
                ->accept(Request::MIMETYPE_JSON)
                ->send().json();
        user_display_name = response_json.at("name");
    } catch (...) {
        BoxCloud::handleExceptions(std::current_exception(), "");
    }
    return user_display_name;
}

void BoxCloud::logout() {
    // logout without providing a client-Id seems to be not supported:
    // https://developer.box.com/guides/authentication/tokens/revoke/
    m_request->reset_auth();
}
