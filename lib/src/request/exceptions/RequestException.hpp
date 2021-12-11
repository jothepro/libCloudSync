#pragma once

#include <stdexcept>

namespace CloudSync::request::exceptions {
    class RequestException : public std::runtime_error {
    public:
        explicit RequestException(const std::string &what) : std::runtime_error(what) {};
    };
}