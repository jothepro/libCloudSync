#pragma once

#include "File.hpp"
#include "Resource.hpp"
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace CloudSync {
/**
 * Provider-independend representation of a directory. Can be seen as a pointer to the actual folder in the cloud. This
 * does not hold the contents of the folder in memory but makes a network call each time you ask for a resource.
 */
class Directory : public Resource {
  public:
    /**
     * list the current directories content.
     * @warning Don't use this to just check if a single file or folder exists! Use `cd()` or `file()` for that and
     * catch the Resource::NoSuchFileOrDirectory exception.
     * @return a list of resources (Files & Folders) that are contained in the folder.
     */
    virtual std::vector<std::shared_ptr<Resource>> ls() const = 0;
    /**
     * change directory
     * @note The provided path will **always** be handled as relative path. Leading slashes will be ignored. If you need
     * to pass an absolute path you should do this:
     *       @code
     *       cloud->root()->cd("some/absolute/path");
     *       @endcode
     * @param path relative path to any other folder. Example values:
     *          * `foldername`
     *          * `path/to/other/folder`
     *          * `..` change to upper directory
     *          * `subfolder/..` returns the current folder
     *          * `/foldername/` trailing & leading slashes are ignored. Same as: `foldername`
     * @return the desired directory
     */
    virtual std::shared_ptr<Directory> cd(const std::string &path) const = 0;
    /**
     * remove folder.
     *
     * @bug If the folder cannot be removed because it still contains resources, this fails with an undefined behaviour.
     *      It may for example throw a Cloud::CommunicationError.
     *      [Help me to improve this](https://gitlab.com/jothepro/libcloudsync)
     */
    virtual void rmdir() const = 0;

    /**
     * print working directory. alias for `folder->path`.
     * @return the absolute path of the folder without trailing slash.
     */
    virtual std::string pwd() const = 0;
    /**
     * create a new directory
     * @param path to the new folder. Does not create intermediate directories.
     * @return the newly created folder
     */
    virtual std::shared_ptr<Directory> mkdir(const std::string &path) const = 0;
    /**
     * create a new file
     * @param path to the new file.
     * @return the newly created file
     */
    virtual std::shared_ptr<File> touch(const std::string &path) const = 0;
    /**
     * get file from path
     * @param path to the file. The file must already exist.
     * @return the requested file
     */
    virtual std::shared_ptr<File> file(const std::string &path) const = 0;
};
} // namespace CloudSync
