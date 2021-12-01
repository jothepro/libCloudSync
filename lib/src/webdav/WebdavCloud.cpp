#include "WebdavCloud.hpp"

using namespace CloudSync;
using namespace CloudSync::webdav;

void WebdavCloud::handleExceptions(const std::exception_ptr &e, const std::string &resourcePath) {
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

void WebdavCloud::logout() {
    // reset login credentials
    m_request->reset_auth();
}
