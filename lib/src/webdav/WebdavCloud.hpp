#pragma once

#include "CloudImpl.hpp"
#include "CloudSync/Exceptions.hpp"
#include "request/Request.hpp"
#include "WebdavDirectory.hpp"
#include <algorithm>

namespace CloudSync::webdav {
class WebdavCloud : public CloudImpl {
  public:
    WebdavCloud(const std::string &url, const std::shared_ptr<request::Request> &request) : CloudImpl(url, request) {}

    std::string getAuthorizeUrl() const override {
        return "";
    }

    std::string getTokenUrl() const override {
        return "";
    }

    std::shared_ptr<Directory> root() const override {
        return std::make_shared<WebdavDirectory>(this->baseUrl, "", "/", this->request, "");
    }

    static void handleExceptions(const std::exception_ptr &e, const std::string &resourcePath) {
        try {
            std::rethrow_exception(e);
        } catch (request::Response::NotFound &e) {
            throw Resource::NoSuchFileOrDirectory(resourcePath);
        } catch (request::Response::Forbidden &e) {
            throw Resource::PermissionDenied(resourcePath);
        } catch (request::Response::Unauthorized &e) {
            throw Cloud::AuthorizationFailed();
        } catch (request::Response::ResponseException &e) {
            throw Cloud::CommunicationError(e.what());
        } catch (request::Request::RequestException &e) {
            throw Cloud::CommunicationError(e.what());
        } catch (request::Response::ParseError &e) {
            throw Cloud::InvalidResponse(e.what());
        }
    }

    std::string getUserDisplayName() const override {
        return this->request->getUsername();
    };
};
} // namespace CloudSync::webdav
