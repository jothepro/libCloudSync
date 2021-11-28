#pragma once

#include "Resource.hpp"
#include <iomanip>
#include <sstream>
#include <utility>

namespace CloudSync {
    namespace request {
        class Request;
    }
    class File : public Resource {
    public:

        /**
         * @note a revision is what would be called [ETag](https://en.wikipedia.org/wiki/HTTP_ETag) in the HTTP-Protocol.
         *       It's a unique identifier for the current file version that changes with every change of the file.
         * @return the revision/etag of the file.
         */
        [[nodiscard]] virtual std::string revision() const = 0;

        /**
         * @warning Don't be fooled by the return type of `std::string`. This may also be binary data and it is up to you to
         * handle the file content correctly. Some providers also provide mimetypes for files. That information cannot be
         * exposed in this API as mimetypes are not supported by all providers.
         * @return the file content as a string.
         */
        [[nodiscard]] virtual std::string read_as_string() const = 0;

        /**
         * @throws Resource::ResourceHasChanged if the file has changed on the server. Check for a new file version with
         * `poll_change()` to resolve this exception.
         * @param content New content that should be written to the file. Overrides the existing file content. This may also
         * be binary data.
         */
        virtual void write_string(const std::string &content) = 0;

        /**
         * Checks for a new file version. If a new version exists, the revision of the file will be updated.
         * @param longPoll wether or not to do a longPoll.
         * @note Some providers (Dropbox) support long-polling to give instant feedback when a file is updated on the
         * server. Long-polling is a blocking operation and will return once a new change is available or
         * the request has timed out.
         * @throws Cloud::MethodNotSupportedError if long-polling is not supported by the provider but `longPoll=true`.
         * @return `true` if a new version exists, otherwise `false`
         */
        virtual bool poll_change(bool longPoll = false) = 0;

        /**
         * @return `true` if long-polling is supported, otherwise `false`
         * @code
         * // Example usage: Only longpoll if it's supported
         * file->poll_change(file->supports_long_poll());
         * @endcode
         */
        [[nodiscard]] virtual bool supports_long_poll() const = 0;
    };
} // namespace CloudSync
