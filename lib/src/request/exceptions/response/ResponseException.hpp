#pragma once

#include "request/exceptions/ParseError.hpp"
#include <stdexcept>
#include <string>
#include <nlohmann/json.hpp>
#include <pugixml.hpp>

namespace CloudSync::request::exceptions::response {
    class ResponseException : public std::runtime_error {
    public:
        ResponseException(long code, std::string data, const std::string &what = "<unknown>")
                : std::runtime_error(std::to_string(code) + " " + what), code(code), data(std::move(data)) {};

        const long code;
        const std::string data;

        [[nodiscard]] nlohmann::json json() const {
            try {
                return nlohmann::json::parse(data);
            } catch(const nlohmann::json::exception& e) {
                throw ParseError(e.what());
            }
        }

        [[nodiscard]] std::shared_ptr<pugi::xml_document> xml() const {
            const auto doc = std::make_shared<pugi::xml_document>();
            pugi::xml_parse_result result = doc->load_string(data.data());
            if (result) {
                return doc;
            } else {
                throw ParseError(result.description());
            }
        }
    };

    class ClientError : public ResponseException {
    public:
        ClientError(long code, const std::string &data = "", const std::string &what = "<unknown client error>")
                : ResponseException(code, data, what) {};
    };

    class ServerError : public ResponseException {
    public:
        ServerError(long code, const std::string &data = "", const std::string &what = "<unknown server error>")
                : ResponseException(code, data, what) {};
    };

    class BadRequest : public ClientError {
    public:
        explicit BadRequest(const std::string &data = "") : ClientError(400, data,  "Bad Request\n" + data) {};
    };

    class Unauthorized : public ClientError {
    public:
        explicit Unauthorized(const std::string &data = "") : ClientError(401, data, "Unauthorized\n" + data) {};
    };

    /// 403 Forbidden
    class Forbidden : public ClientError {
    public:
        explicit Forbidden(const std::string &data = "") : ClientError(403, data, "Forbidden\n" + data) {};
    };

    /// 404 Not Found
    class NotFound : public ClientError {
    public:
        explicit NotFound(const std::string &data = "") : ClientError(404, data, "Not Found\n" + data) {};
    };

    /// 405 Method Not Allowed
    class MethodNotAllowed : public ClientError {
    public:
        explicit MethodNotAllowed(const std::string &data = "") : ClientError(405, data, "Method Not Allowed\n" + data) {};
    };

    /// 412 Precondition Failed
    class PreconditionFailed : public ClientError {
    public:
        explicit PreconditionFailed(const std::string &data = "") : ClientError(412, data, "Precondition Failed\n" + data) {};
    };

    /// 409 Conflict
    class Conflict : public ClientError {
    public:
        explicit Conflict(const std::string &data = "") : ClientError(409, data, "Conflict\n" + data) {};
    };

    class InternalServerError : public ServerError {
    public:
        explicit InternalServerError(const std::string &data = "") : ServerError(500, data, "Internal Server Error\n" + data) {};
    };

    class ServiceUnavailable : public ServerError {
    public:
        explicit ServiceUnavailable(const std::string &data = "") : ServerError(503, data, "Service Unavailable\n" + data) {};
    };
}