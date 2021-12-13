#pragma once

#include "Resource.hpp"

namespace CloudSync {
    class File : public Resource {
    public:

        /**
         * @note a revision is what would be called [ETag](https://en.wikipedia.org/wiki/HTTP_ETag) in the HTTP-Protocol.
         *       It's a unique identifier for the current file version that changes with every change of the file.
         * @return the revision/etag of the file.
         */
        [[nodiscard]] virtual std::string revision() const = 0;

        /**
         * @return the file content as a string.
         */
        [[nodiscard]] virtual std::string read() const = 0;
        [[nodiscard]] virtual std::vector<std::uint8_t> read_binary() const = 0;

        /**
         * @throws Resource::ResourceHasChanged if the file has changed on the server. Check for a new file version with
         * `poll_change()` to resolve this exception.
         * @param content New content that should be written to the file. Overrides the existing file content. This may also
         * be binary data.
         */
        virtual void write(const std::string& content) = 0;
        virtual void write_binary(const std::vector<std::uint8_t>& content) = 0;

        /**
         * Checks for a new file version. If a new version exists, the revision of the file will be updated.
         * @return `true` if a new version exists, otherwise `false`
         */
        virtual bool poll_change() = 0;
    };
} // namespace CloudSync
