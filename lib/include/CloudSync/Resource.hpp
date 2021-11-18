#pragma once

#include "Exceptions.hpp"
#include <memory>
#include <string>
#include <utility>

namespace CloudSync {
namespace request {
class Request;
}
class Resource {
  public:
    class ResourceException : public BaseException {
      public:
        explicit ResourceException(const std::string &what) : BaseException(what){};
    };

    class NoSuchFileOrDirectory : public ResourceException {
      public:
        explicit NoSuchFileOrDirectory(const std::string &path) : ResourceException("No such file or directory: " + path){};
    };

    class PermissionDenied : public ResourceException {
      public:
        explicit PermissionDenied(const std::string &path)
            : ResourceException("Forbidden action on file or directory: " + path){};
    };

    class ResourceHasChanged : public ResourceException {
      public:
        explicit ResourceHasChanged(const std::string &path) : ResourceException("Resource has changed: " + path){};
    };

    virtual ~Resource() = default;;
    const std::string name;
    const std::string path;

    /**
     * Wether the Resource is a file or a directory.
     * @return true if resource is a file, false if it is a directory.
     * @note Use this if you need to cast a resource to it's special type to do perform some type-specific action on it.
     *       A resource can not be of any other type than `File` or `Folder`, so you can safely determine it's type with
     * just this method.
     */
    virtual bool isFile() = 0;

    friend std::ostream &operator<<(std::ostream &output, const Resource *resource) {
        output << resource->describe();
        return output;
    }

  protected:
    Resource(
        std::string baseUrl,
        std::string workingDir,
        std::shared_ptr<request::Request> request,
        std::string name)
        : name(std::move(name))
        , path(std::move(workingDir))
        , _baseUrl(std::move(baseUrl))
        , request(std::move(request)){};

    virtual std::string describe() const = 0;

    // MARK: - properties
    const std::string _baseUrl;
    std::shared_ptr<request::Request> request;
};
} // namespace CloudSync
