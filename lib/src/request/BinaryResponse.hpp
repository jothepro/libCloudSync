#pragma once

#include "Response.hpp"
#include "request/exceptions/response/ResponseException.hpp"
#include <utility>
#include <vector>

namespace CloudSync::request {
    class BinaryResponse : public Response {
    public:
        explicit BinaryResponse(long code,
                       std::vector<std::uint8_t>data_param = {},
                       const std::string& content_type = "",
                       const std::unordered_map<std::string, std::string>& headers = {})
                       : Response(code, content_type, headers)
                       , data(std::move(data_param)) {
            handle_error_code(code, [this](){
                return std::string(data.begin(), data.end());
            });
        };
        const std::vector<std::uint8_t> data;
    };
}