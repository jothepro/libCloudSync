#include "WebdavCloud.hpp"

using namespace CloudSync;

void webdav::WebdavCloud::handleExceptions(const std::exception_ptr &e, const std::string &resourcePath) {
    try {
        std::rethrow_exception(e);
    } catch(request::Response::PreconditionFailed &e) {
        throw Resource::ResourceHasChanged(resourcePath);
    } catch (request::Response::NotFound &e) {
        throw Resource::NoSuchResource(resourcePath);
    } catch (request::Response::Forbidden &e) {
        throw Resource::PermissionDenied(resourcePath);
    } catch (request::Response::Unauthorized &e) {
        throw Cloud::AuthorizationFailed();
    } catch (request::Response::ResponseException &e) {
        throw Cloud::CommunicationError(e.what());
    } catch (request::Request::RequestException &e) {
        throw Cloud::CommunicationError(e.what());
    } catch (request::Response::ParseError &e) {
        throw Cloud::InvalidResponse(e.what());
    }
}

void webdav::WebdavCloud::logout() {
    // reset login credentials
    this->request->reset_auth();
}
