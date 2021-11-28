#pragma once

#include "Exceptions.hpp"
#include <memory>
#include <string>
#include <utility>

namespace CloudSync {

    class Resource {
    public:
        class ResourceException : public BaseException {
        public:
            explicit ResourceException(const std::string &what) : BaseException(what) {};
        };

        class NoSuchResource : public ResourceException {
        public:
            explicit NoSuchResource(const std::string &path) : ResourceException(
                    "A resource with the requested type and name doesn't exist: " + path) {};
        };

        class PermissionDenied : public ResourceException {
        public:
            explicit PermissionDenied(const std::string &path)
                    : ResourceException("Forbidden action on file or directory: " + path) {};
        };

        class ResourceHasChanged : public ResourceException {
        public:
            explicit ResourceHasChanged(const std::string &path) : ResourceException(
                    "Resource has changed: " + path) {};
        };

        class ResourceConflict : public ResourceException {
        public:
            explicit ResourceConflict(const std::string &path) : ResourceException(
                    "A resource with this name already exists: " + path) {};
        };

        [[nodiscard]] virtual std::string name() const = 0;

        [[nodiscard]] virtual std::string path() const = 0;

        /**
         * remove this resource.
         *
         * @warning Stop using this resource object after calling `remove()`.
         * You will just get Resource::NoSuchFileOrDirectory exceptions anyway.
         *
         * @note Be aware that deletion a file doesn't always mean it's gone. Most clouds know the concept of a
         * recycle bin and will move deleted resources there.
         *
         * @bug If a directory cannot be removed because it still contains resources, this fails with an undefined behaviour.
         *      It may for example throw a Cloud::CommunicationError.
         *      [Help me to improve this](https://gitlab.com/jothepro/libcloudsync)
         */
        virtual void remove() = 0;

        /**
         * Wether the Resource is a file or a directory.
         * @return true if resource is a file, false if it is a directory.
         * @note Use this if you need to cast a resource to it's special type to do perform some type-specific action on it.
         *       A resource can not be of any other type than `File` or `Directory`, so you can safely determine it's type with
         * just this method.
         */
        [[nodiscard]] virtual bool is_file() const = 0;
    };
} // namespace CloudSync
