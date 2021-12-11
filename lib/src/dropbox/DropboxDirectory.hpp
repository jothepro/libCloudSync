#pragma once

#include "OAuthDirectoryImpl.hpp"
#include "request/Response.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace CloudSync::dropbox {
    class DropboxDirectory : public OAuthDirectoryImpl {
    public:
        DropboxDirectory(const std::string &dir,
                         const std::shared_ptr<credentials::OAuth2CredentialsImpl>& credentials,
                         const std::shared_ptr<request::Request> &request,
                         const std::string &name)
                : OAuthDirectoryImpl("", dir, credentials, request, name){};

        [[nodiscard]] std::vector<std::shared_ptr<Resource>> list_resources() const override;

        [[nodiscard]] std::shared_ptr<Directory> get_directory(const std::filesystem::path &path) const override;

        void remove() override;

        std::shared_ptr<Directory> create_directory(const std::filesystem::path &path) const override;

        std::shared_ptr<File> create_file(const std::filesystem::path &path) const override;

        std::shared_ptr<File> get_file(const std::filesystem::path &path) const override;

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
    };
} // namespace CloudSync::dropbox
