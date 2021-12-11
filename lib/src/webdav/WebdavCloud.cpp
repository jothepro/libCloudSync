#include "WebdavCloud.hpp"


using namespace CloudSync;
using namespace CloudSync::webdav;

void WebdavCloud::logout() {
    // not supported
}

std::string WebdavCloud::get_user_display_name() const {
    return m_credentials->username();
}

std::shared_ptr<Directory> WebdavCloud::root() const {
    return std::make_shared<WebdavDirectory>(
            m_base_url, "", "/",
            m_credentials,
            m_request, "");
}
