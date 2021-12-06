#include "WebdavCloud.hpp"
#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"

using namespace CloudSync;
using namespace CloudSync::webdav;

void WebdavCloud::handleExceptions(const std::exception_ptr &e, const std::string &resourcePath) {
    try {
        std::rethrow_exception(e);
    } catch(request::Response::PreconditionFailed &e) {
        throw exceptions::resource::ResourceHasChanged(resourcePath);
    } catch (request::Response::NotFound &e) {
        throw exceptions::resource::NoSuchResource(resourcePath);
    } catch (request::Response::Forbidden &e) {
        throw exceptions::resource::PermissionDenied(resourcePath);
    } catch (request::Response::Unauthorized &e) {
        throw exceptions::cloud::AuthorizationFailed();
    } catch (request::Response::ResponseException &e) {
        throw exceptions::cloud::CommunicationError(e.what());
    } catch (request::Request::RequestException &e) {
        throw exceptions::cloud::CommunicationError(e.what());
    } catch (request::Response::ParseError &e) {
        throw exceptions::cloud::InvalidResponse(e.what());
    }
}

void WebdavCloud::logout() {
    // not supported
}
