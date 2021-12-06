#pragma once

#include <stdexcept>
#include <string>

namespace CloudSync::exceptions {
    /**
     * @brief Super-exception for all logical exceptions in this library.
     *
     * If anything goes wrong while accessing the cloud & its resources, an
     * exception inherited from this one will be thrown.
     */
    class Exception : public std::runtime_error {
    public:
        explicit Exception(const std::string &what) : std::runtime_error(what) {};
    };


} // namespace CloudSync
