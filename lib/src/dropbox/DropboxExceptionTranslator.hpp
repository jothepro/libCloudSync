#pragma once

#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"
#include "request/exceptions/RequestException.hpp"
#include "request/exceptions/response/ResponseException.hpp"
#include "request/exceptions/ParseError.hpp"
#include <exception>

namespace CloudSync::dropbox {
    class DropboxExceptionTranslator {
    public:
        static void translate(const std::filesystem::path& path = "") {
            try {
                std::rethrow_exception(std::current_exception());
            } catch (request::exceptions::response::Conflict &e) {
                try {
                    const auto error_json = e.json();
                    if ((to_string(error_json.at("error_summary")).find("conflict") != std::string::npos)) {
                        throw exceptions::resource::ResourceConflict(e.what());
                    } else if ((to_string(error_json.at("error_summary")).find("not_found") != std::string::npos)) {
                        throw exceptions::resource::NoSuchResource(path);
                    } else {
                        throw exceptions::cloud::CommunicationError(e.what());
                    }
                } catch (nlohmann::json::exception &e) {
                    throw exceptions::cloud::InvalidResponse(e.what());
                } catch (request::exceptions::ParseError &e) {
                    throw exceptions::cloud::InvalidResponse(e.what());
                }
            } catch (request::exceptions::response::Unauthorized &e) {
                throw exceptions::cloud::AuthorizationFailed();
            } catch (request::exceptions::response::ResponseException &e) {
                throw exceptions::cloud::CommunicationError(e.what());
            } catch (nlohmann::json::exception &e) {
                throw exceptions::cloud::InvalidResponse(e.what());
            } catch (request::exceptions::ParseError &e) {
                throw exceptions::cloud::InvalidResponse(e.what());
            } catch (request::exceptions::RequestException &e) {
                throw exceptions::cloud::CommunicationError(e.what());
            }
        }
    };
}