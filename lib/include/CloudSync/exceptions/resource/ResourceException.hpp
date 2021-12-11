#pragma once

#include "CloudSync/exceptions/Exception.hpp"

#include <utility>
#include <filesystem>

namespace CloudSync::exceptions::resource {
    /// Base-exception for all resource-related errors.
    class ResourceException : public Exception {
    public:
        explicit ResourceException(const std::string &what, std::filesystem::path path)
        : Exception(what)
        , path(std::move(path)) {};
        const std::filesystem::path path;
    };

    /**
     * @brief A resource with the requested type and name doesn't exist.
     */
    class NoSuchResource : public ResourceException {
    public:
        explicit NoSuchResource(const std::filesystem::path &path)
        : ResourceException("A resource with the requested type and name doesn't exist: " + path.generic_string(), path) {};
    };

    /**
     * @brief Forbidden action on a file or directory.
     */
    class PermissionDenied : public ResourceException {
    public:
        explicit PermissionDenied(const std::filesystem::path &path)
        : ResourceException("Forbidden action on file or directory: " + path.generic_string(), path) {};
    };

    /**
     * @brief The resource has changed on the server.
     */
    class ResourceHasChanged : public ResourceException {
    public:
        explicit ResourceHasChanged(const std::filesystem::path &path)
        : ResourceException("Resource has changed: " + path.generic_string(), path) {};
    };

    /**
     * @brief A file or directory with this name already exists.
     */
    class ResourceConflict : public ResourceException {
    public:
        explicit ResourceConflict(const std::filesystem::path &path)
        : ResourceException("A resource with this name already exists: " + path.generic_string(), path) {};
    };
}