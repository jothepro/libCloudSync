#pragma once

#include "CloudSync/exceptions/Exception.hpp"

namespace CloudSync::exceptions::resource {
    /// Base-exception for all resource-related errors.
    class ResourceException : public Exception {
    public:
        explicit ResourceException(const std::string &what) : Exception(what) {};
    };

    /**
     * @brief A resource with the requested type and name doesn't exist.
     */
    class NoSuchResource : public ResourceException {
    public:
        explicit NoSuchResource(const std::string &path) : ResourceException(
                "A resource with the requested type and name doesn't exist: " + path) {};
    };

    /**
     * @brief Forbidden action on a file or directory.
     */
    class PermissionDenied : public ResourceException {
    public:
        explicit PermissionDenied(const std::string &path)
                : ResourceException("Forbidden action on file or directory: " + path) {};
    };

    /**
     * @brief The resource has changed on the server.
     */
    class ResourceHasChanged : public ResourceException {
    public:
        explicit ResourceHasChanged(const std::string &path) : ResourceException(
                "Resource has changed: " + path) {};
    };

    /**
     * @brief A file or directory with this name already exists.
     */
    class ResourceConflict : public ResourceException {
    public:
        explicit ResourceConflict(const std::string &path) : ResourceException(
                "A resource with this name already exists: " + path) {};
    };
}