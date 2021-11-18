#pragma once

#include "CloudSync/Directory.hpp"
#include "request/Response.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace CloudSync::dropbox {
class DropboxDirectory : public Directory {
  public:
    DropboxDirectory(const std::string &dir, const std::shared_ptr<request::Request> &request, const std::string &name)
        : Directory("", dir, request, name){};
    std::vector<std::shared_ptr<Resource>> ls() const override;
    std::shared_ptr<Directory> cd(const std::string &path) const override;
    void rmdir() const override;
    std::shared_ptr<Directory> mkdir(const std::string &path) const override;
    std::shared_ptr<File> touch(const std::string &path) const override;
    std::shared_ptr<File> file(const std::string &path) const override;

  private:
    /**
     * Takes a json object describing a dropbox resource and converts it into a
     * Resource object.
     *
     * @param resourceTypeFallback [""|"folder"|"file"] If the type fallback is
     * provided and the json object **does not** contain any type information,
     * the fallback type will decide on the resulting Resource type. If the json
     * object **does** contains type information, it will be validated against
     * the fallback type. The parsing will fail if they don't match up.
     */
    std::shared_ptr<Resource> parseEntry(const json &entry, const std::string &resourceTypeFallback = "") const;

    /**
     * Parses a new path-string to make it a valid path that can be passed to
     * dropbox.
     *
     * Normalizes the path (removes all clutter like `//`, `/..`) and removes
     * any trailing `/` if present, because dropbox does not accept paths with
     * trailing `/`
     * @param path a path representation
     * @return a normalized path
     */
    static std::string parsePath(const std::string &path);
};
} // namespace CloudSync::dropbox