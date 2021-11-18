#pragma once

#include <stdexcept>

namespace CloudSync {
/**
 * This is the super-exception for all logical cloud exceptions.
 *
 * If anything goes wrong while accessing the cloud & its resources, an
 * exception inherited from this one will be thrown.
 */
    class BaseException : public std::runtime_error {
    public:
        explicit BaseException(const std::string &what) : std::runtime_error(what) {};
    };
} // namespace CloudSync
