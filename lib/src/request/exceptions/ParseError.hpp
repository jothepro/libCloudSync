#pragma once

#include <stdexcept>
#include <string>

namespace CloudSync::request::exceptions {
    class ParseError : public std::runtime_error {
    public:
        explicit ParseError(const std::string &details) : std::runtime_error("response could not be parsed: " + details) {};
    };
}