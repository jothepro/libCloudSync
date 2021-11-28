#pragma once

#include "File.hpp"
#include "Resource.hpp"
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace CloudSync {
/**
 * Provider-independent representation of a directory. Can be seen as a pointer to the actual folder in the cloud. This
 * does not hold the contents of the folder in memory but makes a network call each time you ask for a resource.
 */
    class Directory : public Resource {
    public:
        /**
         * list the current directories content.
         * @warning Don't use this to just check if a single file or folder exists! Use `get_directory()` or `get_file()` for that and
         * catch the Resource::NoSuchFileOrDirectory exception.
         * @return a list of resources (files & directories) that are contained in this directory.
         */
        [[nodiscard]] virtual std::vector<std::shared_ptr<Resource>> list_resources() const = 0;

        /**
         * change directory
         * @note The provided path will **always** be handled as relative path. Leading slashes will be ignored. If you need
         * to pass an absolute path you should do this:
         *       @code
         *       cloud->root()->get_directory("some/absolute/path");
         *       @endcode
         * @param path relative path to any other folder. Example values:
         *          * `foldername`
         *          * `path/to/other/folder`
         *          * `..` change to upper directory
         *          * `subfolder/..` returns the current folder
         *          * `foldername/` leading slashes are ignored. Same as: `foldername`
         * @return the desired directory
         */
        virtual std::shared_ptr<Directory> get_directory(const std::string &path) const = 0;

        /**
         * Create a new directory.
         * @param path to the new folder. Intermediates are created if they don't exist.
         * @return the requested directory
         */
        virtual std::shared_ptr<Directory> create_directory(const std::string &path) const = 0;

        /**
         * Create a new file.
         * @param path to the new file. Intermediate folders are created if they don't exist.
         * @return the newly created file
         */
        virtual std::shared_ptr<File> create_file(const std::string &path) const = 0;

        /**
         * get file from path
         * @param path to the file. The file must already exist.
         * @return the requested file
         */
        virtual std::shared_ptr<File> get_file(const std::string &path) const = 0;
    };
} // namespace CloudSync
