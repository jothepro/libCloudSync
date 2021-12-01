#pragma once

#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <sstream>

namespace CloudSync::request {
    class Response {
    public:
        // MARK: - exceptions

        class ParseError : public std::runtime_error {
        public:
            ParseError(const std::string &details) : std::runtime_error("response could not be parsed: " + details) {};
        };

        class ResponseException : public std::runtime_error {
        public:
            ResponseException(const std::string &data, int code, const std::string &what = "<unknown>")
                    : std::runtime_error(std::to_string(code) + " " + what), _code(code), _data(data) {};

            int code() const {
                return _code;
            }

            std::string data() const {
                return _data;
            }

        private:
            int _code;
            std::string _data;
        };

        class ClientError : public ResponseException {
        public:
            ClientError(const std::string &data, int code, const std::string &what = "<unknown client error>")
                    : ResponseException(data, code, what) {};
        };

        class ServerError : public ResponseException {
        public:
            ServerError(const std::string &data, int code, const std::string &what = "<unknown server error>")
                    : ResponseException(data, code, what) {};
        };

        class BadRequest : public ClientError {
        public:
            BadRequest(const std::string &data) : ClientError(data, 400, "Bad Request\n" + data) {};
        };

        class Unauthorized : public ClientError {
        public:
            Unauthorized(const std::string &data) : ClientError(data, 401, "Unauthorized\n" + data) {};
        };

        class Forbidden : public ClientError {
        public:
            Forbidden(const std::string &data) : ClientError(data, 403, "Forbidden\n" + data) {};
        };

        class NotFound : public ClientError {
        public:
            NotFound(const std::string &data) : ClientError(data, 404, "Not Found\n" + data) {};
        };

        class MethodNotAllowed : public ClientError {
        public:
            MethodNotAllowed(const std::string &data) : ClientError(data, 405, "Method Not Allowed\n" + data) {};
        };

        class PreconditionFailed : public ClientError {
        public:
            PreconditionFailed(const std::string &data) : ClientError(data, 412, "Precondition Failed\n" + data) {};
        };

        class Conflict : public ClientError {
        public:
            Conflict(const std::string &data) : ClientError(data, 409, "Conflict\n" + data) {};
        };

        class InternalServerError : public ServerError {
        public:
            InternalServerError(const std::string &data) : ServerError(data, 500, "Internal Server Error\n" + data) {};
        };

        class ServiceUnavailable : public ServerError {
        public:
            ServiceUnavailable(const std::string &data) : ServerError(data, 503, "Service Unavailable\n" + data) {};
        };

        // MARK: - properties
        const long code;
        const std::string data;
        const std::string contentType;
        const std::unordered_map<std::string, std::string> headers;

        nlohmann::json json() {
            nlohmann::json response_json;
            try {
                response_json = nlohmann::json::parse(std::istringstream(this->data));
            } catch (...) {
                throw ParseError("invalid json");
            }
            return response_json;
        }

        std::shared_ptr<pugi::xml_document> xml() {
            const auto doc = std::make_shared<pugi::xml_document>();
            pugi::xml_parse_result result = doc->load_string(this->data.c_str());
            if (result) {
                return doc;
            } else {
                throw ParseError("invalid xml");
            }
        }

        // MARK: - constructor
        Response(
                long code, const std::string &data = "", const std::string &contentType = "",
                const std::unordered_map<std::string, std::string> &headers = {})
                : code(code), data(data), contentType(contentType), headers(headers) {
            if (code >= 400) {
                switch (code) {
                    case 400:
                        throw BadRequest(data);
                    case 401:
                        throw Unauthorized(data);
                    case 403:
                        throw Forbidden(data);
                    case 404:
                        throw NotFound(data);
                    case 405:
                        throw MethodNotAllowed(data);
                    case 409:
                        throw Conflict(data);
                    case 412:
                        throw PreconditionFailed(data);
                    case 500:
                        throw InternalServerError(data);
                    case 503:
                        throw ServiceUnavailable(data);
                    default:
                        if (code >= 500) {
                            throw ServerError(data, code);
                        } else {
                            throw ClientError(data, code);
                        }
                }
            }
        };
    };
} // namespace CloudSync::request
