#include "WebdavDirectory.hpp"
#include "request/Request.hpp"
#include "request/Response.hpp"
#include "WebdavCloud.hpp"
#include "WebdavFile.hpp"
#include "pugixml.hpp"
#include <filesystem>
#include <sstream>
#include <vector>

using namespace pugi;
using namespace CloudSync::request;
using P = Request::ParameterType;
namespace fs = std::filesystem;

namespace CloudSync::webdav {
std::string WebdavDirectory::xmlQuery = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
                                        "<d:propfind  xmlns:d=\"DAV:\">\n"
                                        "   <d:prop>\n"
                                        "       <d:getlastmodified />\n"
                                        "       <d:getetag />\n"
                                        "       <d:getcontenttype />\n"
                                        "       <d:resourcetype />\n"
                                        "       <d:getcontentlength />\n"
                                        "   </d:prop>\n"
                                        "</d:propfind>";

std::vector<std::shared_ptr<Resource>> WebdavDirectory::ls() const {
    std::vector<std::shared_ptr<Resource>> resources;
    const auto resourcePath = this->requestUrl("");
    try {
        const auto propfindResult =
            this->request
                ->PROPFIND(
                    resourcePath,
                    {{P::HEADERS,
                      {{"Depth", "1"}, {"Accept", Request::MIMETYPE_XML}, {"Content-Type", Request::MIMETYPE_XML}}}},
                    xmlQuery)
                .xml();
        resources = this->parseXmlResponse(propfindResult->root());
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return resources;
}
std::shared_ptr<Directory> WebdavDirectory::cd(const std::string &path) const {
    const auto resourcePath = this->requestUrl(path);
    std::shared_ptr<WebdavDirectory> directory;
    try {
        const auto propfindResponse =
            this->request
                ->PROPFIND(
                    resourcePath,
                    {{P::HEADERS,
                      {{"Depth", "0"}, {"Accept", Request::MIMETYPE_XML}, {"Content-Type", Request::MIMETYPE_XML}}}},
                    WebdavDirectory::xmlQuery)
                .xml();
        const auto resourceList = this->parseXmlResponse(propfindResponse->root());
        if (resourceList.size() == 1) {
            directory = std::dynamic_pointer_cast<WebdavDirectory>(resourceList[0]);
        } else {
            throw Cloud::CommunicationError("cannot get resource description");
        }
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return directory;
}

void WebdavDirectory::rmdir() const {
    const std::string resourcePath = this->requestUrl("");
    try {
        this->request->DELETE(resourcePath);
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
    }
}
std::shared_ptr<Directory> WebdavDirectory::mkdir(const std::string &name) const {
    const auto resourcePath = this->requestUrl(name);
    std::shared_ptr<WebdavDirectory> directory;
    try {
        this->request->MKCOL(resourcePath);
        const auto propfindResponse =
            this->request
                ->PROPFIND(
                    resourcePath,
                    {{P::HEADERS,
                      {{"Depth", "0"}, {"Accept", Request::MIMETYPE_XML}, {"Content-Type", Request::MIMETYPE_XML}}}},
                    WebdavDirectory::xmlQuery)
                .xml();
        const auto resourceList = this->parseXmlResponse(propfindResponse->root());
        if (resourceList.size() == 1) {
            directory = std::dynamic_pointer_cast<WebdavDirectory>(resourceList[0]);
        } else {
            throw Cloud::CommunicationError("cannot get resource description");
        }

    } catch (Response::Conflict &e) {
        throw NoSuchFileOrDirectory(resourcePath);
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return directory;
}

std::shared_ptr<File> WebdavDirectory::touch(const std::string &name) const {
    const auto resourcePath = this->requestUrl(name);
    std::shared_ptr<File> file;
    try {
        this->request->PUT(resourcePath, {{P::HEADERS, {{"Content-Type", Request::MIMETYPE_BINARY}}}}, "");
        file = this->file(name);
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return file;
}

std::shared_ptr<File> WebdavDirectory::file(const std::string &name) const {
    const auto resourcePath = this->requestUrl(name);
    std::shared_ptr<WebdavFile> file;
    try {
        const auto propfindResponse =
            this->request
                ->PROPFIND(
                    resourcePath,
                    {{P::HEADERS,
                      {{"Depth", "0"}, {"Accept", Request::MIMETYPE_XML}, {"Content-Type", Request::MIMETYPE_XML}}}},
                    WebdavDirectory::xmlQuery)
                .xml();
        const auto resourceList = this->parseXmlResponse(propfindResponse->root());
        if (resourceList.size() == 1) {
            file = std::dynamic_pointer_cast<WebdavFile>(resourceList[0]);
        } else {
            throw Cloud::CommunicationError("cannot get metadata for file");
        }
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return file;
}

std::vector<std::shared_ptr<Resource>> WebdavDirectory::parseXmlResponse(const xml_node &response) const {
    std::vector<std::shared_ptr<Resource>> resources;
    const auto responseNodeSets = response.select_nodes("/*[local-name()='multistatus']/*[local-name()='response']");
    for (const auto responseNodeSet : responseNodeSets) {
        const auto responseNode = responseNodeSet.node();

        // read path from xml
        std::string resourceHref = responseNode.select_node("./*[local-name()='href']").node().child_value();
        // remove path offset from the beginning of the path
        resourceHref.erase(0, this->dirOffset.size());
        // remove any trailing slashes because webdav returns folders with
        // trailing slashes.
        WebdavDirectory::removeTrailingSlashes(resourceHref);
        if (resourceHref != this->path) {
            // parse the href as path so the filename/foldername can be
            // extracted
            const auto resourcePath = fs::path(resourceHref);
            std::shared_ptr<Resource> resource;
            // if the collection node exists, we can be sure this is a
            // directory, else it must be a file
            const auto filename = resourcePath.filename().string();
            if (responseNode.select_node("./*[local-name()='propstat']"
                                         "/*[local-name()='prop']"
                                         "/*[local-name()='resourcetype']"
                                         "/*[local-name()='collection']")) {

                resource = std::make_shared<WebdavDirectory>(
                    this->_baseUrl,
                    this->dirOffset,
                    resourceHref,
                    this->request,
                    filename);
            } else if (
                const auto revision = responseNode
                                          .select_node("./*[local-name()='propstat']"
                                                       "/*[local-name()='prop']"
                                                       "/*[local-name()='getetag']")
                                          .node()
                                          .child_value()) {
                resource = std::make_shared<WebdavFile>(
                    this->_baseUrl + this->dirOffset,
                    resourceHref,
                    this->request,
                    filename,
                    revision);
            }
            resources.push_back(resource);
        }
    }
    return resources;
}

std::string WebdavDirectory::requestUrl(const std::string &path) const {
    // normalize path
    std::string normalizedPath = (fs::path(this->path) / path).lexically_normal().generic_string();
    // remove any trailing slashes, because we cannot be sure if the user adds
    // them or not
    WebdavDirectory::removeTrailingSlashes(normalizedPath);
    std::stringstream result;
    result << this->_baseUrl << this->dirOffset << normalizedPath;
    return result.str();
}

void WebdavDirectory::removeTrailingSlashes(std::string &path) {
    while (path.size() > 1 && path.back() == '/') {
        path.erase(path.size() - 1);
    }
}
} // namespace CloudSync::webdav
