#pragma once

#include "Cloud.hpp"
#include <memory>

namespace CloudSync {

    class CloudFactory : public std::enable_shared_from_this<CloudFactory> {
    public:
        CloudFactory();

        /**
         * Creates a webdav cloud instance.
         * @code
         * auto cloud = CloudFactory().webdav("https://nextcloud.webo.hosting/remote.php/webdav");
         * @endcode
         * @param url address of the webdav endpoint
         * @param request custom Request interface implementation to be used for networking
         */
        [[nodiscard]] std::shared_ptr<Cloud> webdav(const std::string &url);

        /**
         * Creates a nextcloud cloud instance.
         * @see https://nextcloud.com
         * @code
         * auto cloud = CloudFactory().nextcloud("https://nextcloud.webo.hosting");
         * @endcode
         * @param url address of the nextcloud server.
         * @param request custom Request interface implementation to be used for networking
         * @warning do **not** include the path to the webdav endpoint (`/remote.php/webdav`) in the address. The library
         * will handle this detail for you!
         */
        [[nodiscard]] std::shared_ptr<Cloud> nextcloud(const std::string &url);

        /**
         * Create a dropbox cloud instance.
         * @see https://www.dropbox.com
         * @code
         * auto cloud = CloudFactory().dropbox();
         * @endcode
         * @param request custom Request interface implementation to be used for networking
         */
        [[nodiscard]] std::shared_ptr<Cloud> dropbox();

        /**
         * Create a box cloud instance.
         * @see https://www.box.com
         * @code
         * auto cloud = CloudFactory().box();
         * @endcode
         * @param request custom Request interface implementation to be used for networking
         */
        [[nodiscard]] std::shared_ptr<Cloud> box();

        /**
         * Creates a OneDrive cloud instance
         * @see https://onedrive.live.com
         * @see https://docs.microsoft.com/en-us/onedrive/developer/rest-api/api/drive_get?view=odsp-graph-online
         * @param drive specifies the root of the drive (without leading & trailing slash). Example values:
         *          * `me/drive/root`
         *          * `me/drive/special/approot`
         *          * `groups/{groupId}/drive/root`
         *          * `sites/{siteId}/drive/root`
         *          * `drives/{drive-id}/root`
         * @param request custom Request interface implementation to be used for networking
         */
        [[nodiscard]] std::shared_ptr<Cloud> onedrive(const std::string &drive = "me/drive/root");

        /**
         * Creates a Google Drive cloud instance
         * @see https://drive.google.com/
         * @note Google Drive supports app-data folders. But like with OneDrive you need to explicitly tell the API that you
         * want to access the app-folder. If you set `rootName` to `root` but the user only has permission for the scope
         * `drive.appdata`, the root directory will be empty without write permissions.
         * @param rootName the type of root. Possible values: `root`, `appDataFolder`.
         * @param request custom Request interface implementation to be used for networking
         */
        [[nodiscard]] std::shared_ptr<Cloud> gdrive(const std::string &rootName = "root");

        /**
         * By default the library respects the proxy environment variable `http_proxy`.
         *
         * This method overrides the environment variable with a custom proxy for all network calls.
         * @code
         * // all network calls to onedrive will be made through the applied proxy
         * auto cloud = CloudFactory().proxy("http://proxy:80").onedrive();
         * @endcode
         * @param proxy
         */
        std::shared_ptr<CloudFactory> proxy(const std::string& url, const std::string& username = "", const std::string& password = "");
    private:
        std::shared_ptr<request::Request> requestImplementation;

        std::shared_ptr<request::Request> getRequestImplementation();
    };

} // namespace CloudSync
