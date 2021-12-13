#include "CloudSync/CloudFactory.hpp"
#include "CloudSync/Cloud.hpp"
#include "dropbox/DropboxCloud.hpp"
#include "gdrive/GDriveCloud.hpp"
#include "nextcloud/NextcloudCloud.hpp"
#include "onedrive/OneDriveCloud.hpp"
#include "webdav/WebdavCloud.hpp"
#include "request/curl/CurlRequest.hpp"

using namespace CloudSync;
using namespace CloudSync::credentials;

CloudFactory::CloudFactory() {
    m_request = std::make_shared<request::curl::CurlRequest>();
    m_request->set_follow_redirects(true);
};

std::shared_ptr<Cloud> CloudFactory::create_webdav(const std::string &url, const std::shared_ptr<BasicCredentials>& credentials) {
    return std::make_shared<webdav::WebdavCloud>(url, std::static_pointer_cast<credentials::BasicCredentialsImpl>(credentials), m_request);
}

std::shared_ptr<Cloud> CloudFactory::create_nextcloud(const std::string &url, const std::shared_ptr<BasicCredentials>& credentials) {
    return std::make_shared<nextcloud::NextcloudCloud>(url, std::static_pointer_cast<credentials::BasicCredentialsImpl>(credentials), m_request);
}

std::shared_ptr<Cloud> CloudFactory::create_dropbox(const std::shared_ptr<OAuth2Credentials> & credentials) {
    return std::make_shared<dropbox::DropboxCloud>(std::static_pointer_cast<credentials::OAuth2CredentialsImpl>(credentials), m_request);
}

std::shared_ptr<Cloud> CloudFactory::create_onedrive(const std::shared_ptr<OAuth2Credentials> & credentials, const std::string &drive) {
    return std::make_shared<onedrive::OneDriveCloud>(drive, std::static_pointer_cast<credentials::OAuth2CredentialsImpl>(credentials), m_request);
}

std::shared_ptr<Cloud> CloudFactory::create_gdrive(const std::shared_ptr<OAuth2Credentials> & credentials, const std::string &rootName) {
    return std::make_shared<gdrive::GDriveCloud>(rootName, std::static_pointer_cast<credentials::OAuth2CredentialsImpl>(credentials), m_request);
}

void CloudFactory::set_proxy(const std::string& url, const std::string& username, const std::string& password) {
    m_request->set_proxy(url, username, password);
}
