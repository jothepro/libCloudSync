#pragma once

#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"
#include "request/exceptions/RequestException.hpp"
#include "request/exceptions/response/ResponseException.hpp"
#include "request/exceptions/ParseError.hpp"
#include <exception>

namespace CloudSync::gdrive {
    class GDriveExceptionTranslator {
    public:
        static void translate(const std::filesystem::path& path = "") {
            try {
                std::rethrow_exception(std::current_exception());
            } catch (request::exceptions::response::NotFound &e) {
                try {
                    const auto error_message = e.json();
                    if (error_message.at("error").at("errors").at(0).at("reason") == "notFound") {
                        throw exceptions::resource::NoSuchResource(path);
                    } else {
                        throw exceptions::cloud::CommunicationError("unknown error response");
                    }
                } catch (nlohmann::json::exception &e) {
                    throw exceptions::cloud::InvalidResponse(e.what());
                } catch (request::exceptions::ParseError &e) {
                    throw exceptions::cloud::InvalidResponse(e.what());
                }
            } catch (request::exceptions::response::Unauthorized &e) {
                throw exceptions::cloud::AuthorizationFailed();
            } catch (request::exceptions::response::PreconditionFailed &e) {
                throw exceptions::resource::ResourceHasChanged(path);
            } catch (request::exceptions::response::Forbidden &) {
                throw exceptions::resource::PermissionDenied(path);
            } catch (nlohmann::json::exception &e) {
                throw exceptions::cloud::InvalidResponse(e.what());
            } catch (request::exceptions::RequestException &e) {
                throw exceptions::cloud::CommunicationError(e.what());
            }
        }
    };
}