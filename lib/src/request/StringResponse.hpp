#pragma once

#include "Response.hpp"
#include "request/exceptions/ParseError.hpp"
#include <string>
#include <utility>

namespace CloudSync::request {
    class StringResponse : public Response {
    public:
        explicit StringResponse(long code,
                       std::string data_param = "",
                       const std::string& content_type = "",
                       const std::unordered_map<std::string, std::string>& headers = {})
                       : Response(code, content_type, headers)
                       , data(std::move(data_param)) {
            handle_error_code(code, [this](){
                return data;
            });
        };

        const std::string data;

        [[nodiscard]] nlohmann::json json() const {
            try {
                return nlohmann::json::parse(data);
            } catch(const nlohmann::json::exception& e) {
                throw request::exceptions::ParseError(e.what());
            }
        }

        [[nodiscard]] std::shared_ptr<pugi::xml_document> xml() const {
            const auto doc = std::make_shared<pugi::xml_document>();
            pugi::xml_parse_result result = doc->load_string(data.data());
            if (result) {
                return doc;
            } else {
                throw request::exceptions::ParseError(result.description());
            }
        }
    };
}