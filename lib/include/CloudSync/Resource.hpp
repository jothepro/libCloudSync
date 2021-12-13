#pragma once

#include <memory>
#include <string>
#include <utility>
#include <filesystem>
#include <vector>

namespace CloudSync {
    /// Common interface for both directories and files
    class Resource {
    public:

        [[nodiscard]] virtual std::string name() const = 0;

        [[nodiscard]] virtual std::filesystem::path path() const = 0;

        /**
         * remove this resource.
         *
         * @warning Stop using this resource object after calling `remove()`.
         * You will just get Resource::NoSuchFileOrDirectory exceptions anyway.
         *
         * @note Be aware that deletion a file doesn't always mean it's gone. Most clouds know the concept of a
         * recycle bin and will move deleted resources there.
         *
         * @bug If a directory cannot be removed because it still contains resources, this may fail for some cloud providers.
         *      [Help me to improve this](https://github.com/jothepro/libCloudSync)
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
