#pragma once

#include "Resource.hpp"

namespace CloudSync {
    /**
     * @brief Provider-independent representation of a file.
     *
     * Can be seen as a pointer to the actual file in the cloud. This
     * does not hold the content of the file in memory but makes a network
     * call each time you read the file.
     */
    class File : public Resource {
    public:

        /**
         * @note a revision is what would be called [ETag](https://en.wikipedia.org/wiki/HTTP_ETag) in the HTTP-Protocol.
         *       It's a unique identifier for the current file version that changes with every change of the file.
         * @return the revision/etag of the file.
         */
        [[nodiscard]] virtual std::string revision() const = 0;

        /// Read the content of the file into a string.
        [[nodiscard]] virtual std::string read() const = 0;

        /// Read the content of the file into a binary data-structure.
        [[nodiscard]] virtual std::vector<std::uint8_t> read_binary() const = 0;

        /**
         * @brief Write the content of the file from a string.
         * @throws Resource::ResourceHasChanged if the file has changed on the server. Check for a new file version with
         * `poll_change()` to resolve this exception.
         * @param content New content that should be written to the file. Overrides the existing file content. This may also
         * be binary data.
         */
        virtual void write(const std::string& content) = 0;

        /// Write the content of the file from a binary data-structure.
        virtual void write_binary(const std::vector<std::uint8_t>& content) = 0;

        /**
         * Checks for a new file version. If a new version exists, the revision of the file will be updated.
         * @return `true` if a new version exists, otherwise `false`
         */
        virtual bool poll_change() = 0;
    };
}
