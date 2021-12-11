#pragma once

#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"
#include "request/exceptions/RequestException.hpp"
#include "request/exceptions/response/ResponseException.hpp"
#include "request/exceptions/ParseError.hpp"
#include <exception>

namespace CloudSync::webdav {
    class WebdavExceptionTranslator {
    public:
        static void translate(const std::filesystem::path& path = "") {
            try {
                std::rethrow_exception(std::current_exception());
            } catch(request::exceptions::response::PreconditionFailed &e) {
                throw exceptions::resource::ResourceHasChanged(path);
            } catch (request::exceptions::response::NotFound &e) {
                throw exceptions::resource::NoSuchResource(path);
            } catch (request::exceptions::response::Forbidden &e) {
                throw exceptions::resource::PermissionDenied(path);
            } catch (request::exceptions::response::Unauthorized &e) {
                throw exceptions::cloud::AuthorizationFailed();
            } catch (request::exceptions::response::ResponseException &e) {
                throw exceptions::cloud::CommunicationError(e.what());
            } catch (request::exceptions::RequestException &e) {
                throw exceptions::cloud::CommunicationError(e.what());
            } catch (request::exceptions::ParseError &e) {
                throw exceptions::cloud::InvalidResponse(e.what());
            }
        }
    };
}