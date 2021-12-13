#pragma once

#include "Cloud.hpp"
#include "OAuth2Credentials.hpp"
#include "BasicCredentials.hpp"



namespace CloudSync {
    namespace request {
        class Request;
    }
    /**
     * @brief Entrypoint to the library: Create any cloud instance from a given configuration.
     */
    class CloudFactory {
    public:
        CloudFactory();

        /**
         * @brief Creates a webdav cloud instance.
         * @code
         * auto credentials = BasicCredentials::from_username_password("john", "password123");
         * auto cloud = CloudFactory().create_webdav("https://nextcloud.webo.hosting/remote.php/webdav", credentials);
         * @endcode
         * @param url address of the webdav endpoint
         * @param credentials
         */
        [[nodiscard]] std::shared_ptr<Cloud> create_webdav(const std::string &url, const std::shared_ptr<BasicCredentials>& credentials);

        /**
         * @brief Creates a nextcloud cloud instance.
         * @see https://nextcloud.com
         * @code
         * auto credentials = BasicCredentials::from_username_password("john", "password123");
         * auto cloud = CloudFactory().nextcloud("https://nextcloud.webo.hosting", credentials);
         * @endcode
         * @param url address of the nextcloud server.
         * @param credentials
         * @warning do **not** include the path to the webdav endpoint (`/remote.php/webdav`) in the address. The library
         * will handle this detail for you!
         */
        [[nodiscard]] std::shared_ptr<Cloud> create_nextcloud(const std::string &url, const std::shared_ptr<BasicCredentials>& credentials);

        /**
         * @brief Create a Dropbox cloud instance.
         * @see https://www.dropbox.com
         * @code
         * auto credentials = OAuth2Credentials::from_access_token("r345hreinp5Ec94...");
         * auto cloud = CloudFactory().create_dropbox(credentials);
         * @endcode
         * @param credentials
         */
        [[nodiscard]] std::shared_ptr<Cloud> create_dropbox(const std::shared_ptr<OAuth2Credentials> & credentials);

        /**
         * @brief Creates a OneDrive cloud instance
         * @see https://onedrive.live.com
         * @see https://docs.microsoft.com/en-us/onedrive/developer/rest-api/api/drive_get?view=odsp-graph-online
         * @code
         * auto credentials = OAuth2Credentials::from_access_token("r345hreinp5Ec94...");
         * auto cloud = CloudFactory().create_onedrive(credentials, "me/drive/approot");
         * @endcode
         * @param credentials
         * @param drive specifies the root of the drive (without leading & trailing slash). Example values:
         *          * `me/drive/root`
         *          * `me/drive/special/approot`
         *          * `groups/{groupId}/drive/root`
         *          * `sites/{siteId}/drive/root`
         *          * `drives/{drive-id}/root`
         */
        [[nodiscard]] std::shared_ptr<Cloud> create_onedrive(const std::shared_ptr<OAuth2Credentials> & credentials, const std::string &drive = "me/drive/root");

        /**
         * @brief Creates a Google Drive cloud instance
         * @see https://drive.google.com/
         * @code
         * auto credentials = OAuth2Credentials::from_access_token("r345hreinp5Ec94...");
         * auto cloud = CloudFactory().create_gdrive(credentials, "appDataFolder");
         * @endcode
         * @note Google Drive supports app-data folders. But like with OneDrive you need to explicitly tell the API that you
         * want to access the app-folder. If you set `rootName` to `root` but the user only has permission for the scope
         * `drive.appdata`, the root directory will be empty without write permissions.
         * @param credentials
         * @param rootName the type of root. Possible values: `root`, `appDataFolder`.
         */
        [[nodiscard]] std::shared_ptr<Cloud> create_gdrive(const std::shared_ptr<OAuth2Credentials> & credentials, const std::string &rootName = "root");

        /**
         * @brief Set a custom proxy for all network communication of the cloud instance to be created.
         *
         * By default the library respects the proxy environment variable `http_proxy`.
         *
         * This method overrides the environment variable with a custom proxy for all network calls.
         * @code
         * // all network calls to onedrive will be made through the applied proxy
         * auto cloud = CloudFactory().set_proxy("http://proxy:80").create_onedrive(credentials, "me/drive/root");
         * @endcode
         */
        void set_proxy(const std::string& url, const std::string& username = "", const std::string& password = "");
    private:
        std::shared_ptr<request::Request> m_request;
    };

} // namespace CloudSync
