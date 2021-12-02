#include "CloudSync/CloudFactory.hpp"
#include "CloudSync/Cloud.hpp"
#include "box/BoxCloud.hpp"
#include "dropbox/DropboxCloud.hpp"
#include "gdrive/GDriveCloud.hpp"
#include "nextcloud/NextcloudCloud.hpp"
#include "onedrive/OneDriveCloud.hpp"
#include "webdav/WebdavCloud.hpp"
#include "request/curl/CurlRequest.hpp"

using namespace CloudSync;

CloudFactory::CloudFactory() {
    m_request = std::make_shared<request::curl::CurlRequest>();
    m_request->set_follow_redirects(true);
};

std::shared_ptr<Cloud> CloudFactory::webdav(const std::string &url) {
    return std::make_shared<webdav::WebdavCloud>(url, m_request);
}

std::shared_ptr<Cloud> CloudFactory::nextcloud(const std::string &url) {
    return std::make_shared<nextcloud::NextcloudCloud>(url, m_request);
}

std::shared_ptr<Cloud> CloudFactory::dropbox() {
    return std::make_shared<dropbox::DropboxCloud>(m_request);
}

std::shared_ptr<Cloud> CloudFactory::box() {
    return std::make_shared<box::BoxCloud>(m_request);
}

std::shared_ptr<Cloud> CloudFactory::onedrive(const std::string &drive) {
    return std::make_shared<onedrive::OneDriveCloud>(drive, m_request);
}

std::shared_ptr<Cloud> CloudFactory::gdrive(const std::string &rootName) {
    return std::make_shared<gdrive::GDriveCloud>(rootName, m_request);
}

std::shared_ptr<CloudFactory> CloudFactory::set_proxy(const std::string& url, const std::string& username, const std::string& password) {
    m_request->set_proxy(url, username, password);
    return shared_from_this();
}
