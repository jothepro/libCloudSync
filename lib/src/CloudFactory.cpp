#include "CloudSync/CloudFactory.hpp"
#include "CloudSync/Cloud.hpp"
#include "box/BoxCloud.hpp"
#include "dropbox/DropboxCloud.hpp"
#include "gdrive/GDriveCloud.hpp"
#include "nextcloud/NextcloudCloud.hpp"
#include "onedrive/OneDriveCloud.hpp"
#include "webdav/WebdavCloud.hpp"
#include "request/curl/CurlRequest.hpp"

namespace CloudSync {

    CloudFactory::CloudFactory() = default;


    std::shared_ptr<request::Request> CloudFactory::getRequestImplementation() {
        if (this->requestImplementation == nullptr) {
            this->requestImplementation = std::make_shared<request::curl::CurlRequest>();
        }
        this->requestImplementation->set_follow_redirects(true);
        return this->requestImplementation;
    }

    std::shared_ptr<Cloud> CloudFactory::webdav(const std::string &url) {
        return std::make_shared<webdav::WebdavCloud>(url, this->getRequestImplementation());
    }

    std::shared_ptr<Cloud> CloudFactory::nextcloud(const std::string &url) {
        return std::make_shared<nextcloud::NextcloudCloud>(url, this->getRequestImplementation());
    }

    std::shared_ptr<Cloud> CloudFactory::dropbox() {
        return std::make_shared<dropbox::DropboxCloud>(this->getRequestImplementation());
    }

    std::shared_ptr<Cloud> CloudFactory::box() {
        return std::make_shared<box::BoxCloud>(this->getRequestImplementation());
    }

    std::shared_ptr<Cloud> CloudFactory::onedrive(const std::string &drive) {
        return std::make_shared<onedrive::OneDriveCloud>(drive, this->getRequestImplementation());
    }

    std::shared_ptr<Cloud> CloudFactory::gdrive(const std::string &rootName) {
        return std::make_shared<gdrive::GDriveCloud>(rootName, this->getRequestImplementation());
    }

    std::shared_ptr<CloudFactory> CloudFactory::set_proxy(const std::string& url, const std::string& username, const std::string& password) {
        requestImplementation->set_proxy(url, username, password);
        return shared_from_this();
    }

} // namespace CloudSync
