#pragma once

#include "request/Request.hpp"
#include "pugixml.hpp"
#include "webdav/WebdavCloud.hpp"
#include <string>

using namespace pugi;
using namespace CloudSync::request;
using P = CloudSync::request::Request::ParameterType;

namespace CloudSync::nextcloud {
class NextcloudCloud : public webdav::WebdavCloud {
  public:
    NextcloudCloud(const std::string &url, const std::shared_ptr<request::Request> &request)
        : webdav::WebdavCloud(url, request) {}

    std::string getAuthorizeUrl() const override {
        return this->baseUrl + "/index.php/login/flow";
    }

    std::shared_ptr<Directory> root() const override {
        return std::make_shared<webdav::WebdavDirectory>(this->baseUrl, "/remote.php/webdav", "/", this->request, "");
    }

    std::string getUserDisplayName() const override {
        std::string userDisplayName;
        try {
            const auto responseXml =
                this->request
                    ->GET(
                        this->baseUrl + "/ocs/v1.php/cloud/user",
                        {{P::HEADERS, {{"OCS-APIRequest", "true"}, {"Accept", Request::MIMETYPE_XML}}}})
                    .xml();
            userDisplayName = responseXml->select_node("/ocs/data/display-name").node().child_value();
        } catch (...) {
            NextcloudCloud::handleExceptions(std::current_exception(), "");
        }
        return userDisplayName;
    };
};
} // namespace CloudSync::nextcloud
