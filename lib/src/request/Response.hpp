#pragma once

#include "request/exceptions/response/ResponseException.hpp"
#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <sstream>
#include <utility>

namespace CloudSync::request {
    class Response {
    public:
        Response(long code, std::string  content_type, std::unordered_map<std::string, std::string> headers)
        : code(code)
        , content_type(std::move(content_type))
        , headers(std::move(headers)) {};
        const long code;
        const std::string content_type;
        const std::unordered_map<std::string, std::string> headers;

    protected:
        static void handle_error_code(long code, const std::function<std::string()>& data) {
            if (code >= 400) {                                                                                                 \
                switch (code) {
                    case 400:
                        throw exceptions::response::BadRequest(data());
                    case 401:
                        throw exceptions::response::Unauthorized(data());
                    case 403:
                        throw exceptions::response::Forbidden(data());
                    case 404:
                        throw exceptions::response::NotFound(data());
                    case 405:
                        throw exceptions::response::MethodNotAllowed(data());
                    case 409:
                        throw exceptions::response::Conflict(data());
                    case 412:
                        throw exceptions::response::PreconditionFailed(data());
                    case 500:
                        throw exceptions::response::InternalServerError(data());
                    case 503:
                        throw exceptions::response::ServiceUnavailable(data());
                    default:
                        if (code >= 500) {
                            throw exceptions::response::ServerError(code, data());
                        } else {
                            throw exceptions::response::ClientError(code, data());
                        }
                }
            }
        }
    };
} // namespace CloudSync::request
