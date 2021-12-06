#pragma once

#include "Directory.hpp"

namespace CloudSync {

    class Cloud {
    public:

        /**
         * Use this to check if the connection to the cloud is working. This currently is just a shorthand for
         * `cloud->root()->list_resources();`.
         * @throws AuthorizationFailed if your login-credentials are wrong.
         */
        virtual void test_connection() const = 0;


        [[nodiscard]] virtual std::string get_base_url() const = 0;

        /**
         * Fetches the users display name (!= username, usually its first- & lastname)
         * @warning this makes a network call every time it is called.
         * @return the users display name.
         */
        [[nodiscard]] virtual std::string get_user_display_name() const = 0;

        /**
         * @return root directory. This is the entrypoint for all file operations.
         * @code
         * auto file = cloud->root()->file("path/to/file.txt");
         * @endcode
         */
        [[nodiscard]] virtual std::shared_ptr<Directory> root() const = 0;


        /**
         * Invalidates the login credentials to the currently used cloud, if possible.
         * Resets the credentials after revoking them.
         *
         * - For Dropbox & GDrive the OAuth-token will be revoked.
         * - For Box & OneDrive the token will not be revoked, because this is not supported.
         * - For Nextcloud the app-password will be revoked, if an app password was used.
         * - For Webdav or a Nextcloud-session with a normal password it will do nothing.
         *
         * After a logout you can login again with different credentials.
         *
         * @warning Don't use this Cloud instance after the logout, if you haven't logged in again
         * with different credentials.
         * Any request will fail with a AuthorizationFailed exception.
         *
         * @throws CommunicationError if the request has failed.
         *  You may retry to logout or just proceed to delete the credentials in your application and
         *  ignore the error.
         */
        virtual void logout() = 0;
    };

} // namespace CloudSync
