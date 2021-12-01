#include "WebdavDirectory.hpp"
#include "request/Request.hpp"
#include "request/Response.hpp"
#include "WebdavCloud.hpp"
#include "WebdavFile.hpp"
#include <pugixml.hpp>
#include <filesystem>
#include <sstream>
#include <vector>

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::webdav;
using namespace pugi;
namespace fs = std::filesystem;


std::string WebdavDirectory::xmlQuery =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<d:propfind  xmlns:d=\"DAV:\">"
        "<d:prop>"
            "<d:getlastmodified/>"
            "<d:getetag/>"
            "<d:getcontenttype/>"
            "<d:resourcetype/>"
            "<d:getcontentlength/>"
        "</d:prop>"
    "</d:propfind>";

std::vector<std::shared_ptr<Resource>> WebdavDirectory::list_resources() const {
    std::vector<std::shared_ptr<Resource>> resources;
    const auto resourcePath = this->requestUrl("");
    try {
        const auto response_xml = m_request->PROPFIND(resourcePath)
                ->header("Depth", "1")
                ->accept(Request::MIMETYPE_XML)
                ->content_type(Request::MIMETYPE_XML)
                ->send(xmlQuery).xml();
        resources = this->parseXmlResponse(response_xml->root());
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return resources;
}

std::shared_ptr<Directory> WebdavDirectory::get_directory(const std::string &path) const {
    const auto resourcePath = this->requestUrl(path);
    std::shared_ptr<WebdavDirectory> directory;
    try {
        const auto response_xml = m_request->PROPFIND(resourcePath)
                ->header("Depth", "0")
                ->accept(Request::MIMETYPE_XML)
                ->content_type(Request::MIMETYPE_XML)
                ->send(xmlQuery).xml();
        const auto resourceList = this->parseXmlResponse(response_xml->root());
        if (resourceList.size() == 1) {
            directory = std::dynamic_pointer_cast<WebdavDirectory>(resourceList[0]);
            if(!directory) { // if the dynamic cast has failed, the returned resource is a file, not a directory
                throw Resource::NoSuchResource(resourcePath);
            }
        } else {
            throw Cloud::CommunicationError("cannot get resource description");
        }
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
    }
    return directory;
}

void WebdavDirectory::remove() {
    if(path() != "/") {
        const std::string resourcePath = this->requestUrl("");
        try {
            m_request->DELETE(resourcePath)->send();
        } catch (...) {
            WebdavCloud::handleExceptions(std::current_exception(), resourcePath);
        }
    } else {
        throw PermissionDenied("deleting the root folder is not allowed");
    }

}

std::shared_ptr<Directory> WebdavDirectory::create_directory(const std::string &path) const {
    const auto resourcePath = this->requestUrl(path);
    std::shared_ptr<WebdavDirectory> directory;
    auto normalizedPath = fs::path( WebdavDirectory::remove_trailing_slashes(path));
    if(normalizedPath.has_parent_path()) {
        if(!resource_exists(fs::path(resourcePath).parent_path().generic_string())) {
            create_directory(normalizedPath.parent_path().generic_string());
        }
    }
    try {
        m_request->MKCOL(resourcePath)->send();
        const auto response_xml = m_request->PROPFIND(resourcePath)
                ->header("Depth", "0")
                ->accept(Request::MIMETYPE_XML)
                ->content_type(Request::MIMETYPE_XML)
                ->send(xmlQuery).xml();
        const auto resourceList = this->parseXmlResponse(response_xml->root());
        if (resourceList.size() == 1) {
            directory = std::dynamic_pointer_cast<WebdavDirectory>(resourceList[0]);
        } else {
            throw Cloud::CommunicationError("cannot get resource description");
        }
    } catch(request::Response::MethodNotAllowed &e) {
        throw Resource::ResourceConflict((fs::path(this->path()) / path).generic_string());
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), (fs::path(this->path()) / path).generic_string());
    }
    return directory;
}

std::shared_ptr<File> WebdavDirectory::create_file(const std::string &path) const {
    const auto resourcePath = this->requestUrl(path);
    std::shared_ptr<File> file;
    if(fs::path(path).has_parent_path()) {
        if(!resource_exists(fs::path(resourcePath).parent_path().generic_string())) {
            create_directory(fs::path(path).parent_path().generic_string());
        }
    }
    try {
        if(resource_exists(resourcePath)) {
            throw Resource::ResourceConflict(fs::path(fs::path(this->path()) / path).generic_string());
        } else {
            m_request->PUT(resourcePath)->content_type(Request::MIMETYPE_BINARY)->send();
            file = this->get_file(path);
        }
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), (fs::path(this->path()) / path).generic_string());
    }
    return file;
}

std::shared_ptr<File> WebdavDirectory::get_file(const std::string &path) const {
    const auto resourcePath = this->requestUrl(path);
    std::shared_ptr<WebdavFile> file;
    try {
        const auto response_xml = m_request->PROPFIND(resourcePath)
                ->header("Depth", "0")
                ->accept(Request::MIMETYPE_XML)
                ->content_type(Request::MIMETYPE_XML)
                ->send(WebdavDirectory::xmlQuery).xml();
        const auto resourceList = this->parseXmlResponse(response_xml->root());
        if (resourceList.size() == 1) {
            file = std::dynamic_pointer_cast<WebdavFile>(resourceList[0]);
            if(!file) { // if the dynamic cast has failed, the returned resource is a not a file
                throw Resource::NoSuchResource(resourcePath);
            }
        } else {
            throw Cloud::CommunicationError("cannot get metadata for file");
        }
    } catch (...) {
        WebdavCloud::handleExceptions(std::current_exception(), (fs::path(this->path()) / path).generic_string());
    }
    return file;
}

std::vector<std::shared_ptr<Resource>> WebdavDirectory::parseXmlResponse(const xml_node &response) const {
    std::vector<std::shared_ptr<Resource>> resources;
    const auto responseNodeSets = response.select_nodes(
            "/*[local-name()='multistatus']/*[local-name()='response']");
    for (const auto responseNodeSet: responseNodeSets) {
        const auto responseNode = responseNodeSet.node();

        // read path from xml
        std::string resourceHref = responseNode.select_node("./*[local-name()='href']").node().child_value();
        // remove path offset from the beginning of the path
        resourceHref.erase(0, this->dirOffset.size());
        // remove any trailing slashes because webdav returns folders with
        // trailing slashes.
        resourceHref = WebdavDirectory::remove_trailing_slashes(resourceHref);
        if (resourceHref != this->path()) {
            // parse the href as path so the filename/foldername can be
            // extracted
            const auto resourcePath = fs::path(resourceHref);
            std::shared_ptr<Resource> resource;
            // if the collection node exists, we can be sure this is a
            // directory, else it must be a file
            const auto filename = resourcePath.filename().string();
            if (responseNode.select_node(
                "./*[local-name()='propstat']"
                "/*[local-name()='prop']"
                "/*[local-name()='resourcetype']"
                "/*[local-name()='collection']")
            ) {
                resource = std::make_shared<WebdavDirectory>(
                    m_base_url,
                    this->dirOffset,
                    resourceHref,
                    m_request,
                    filename);
            } else if (const auto revision = responseNode.select_node(
                "./*[local-name()='propstat']"
                "/*[local-name()='prop']"
                "/*[local-name()='getetag']").node().child_value()
            ) {
                resource = std::make_shared<WebdavFile>(
                        m_base_url + this->dirOffset,
                    resourceHref,
                    m_request,
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
    std::string normalized_path = (fs::path(this->path()) / path).lexically_normal().generic_string();
    // remove any trailing slashes, because we cannot be sure if the user adds
    // them or not
    normalized_path = WebdavDirectory::remove_trailing_slashes(normalized_path);
    std::stringstream result;
    result << m_base_url << this->dirOffset << normalized_path;
    return result.str();
}

std::string WebdavDirectory::remove_trailing_slashes(const std::string &path) {
    auto result = path;
    while (result.size() > 1 && result.back() == '/') {
        result.erase(path.size() - 1);
    }
    return result;
}

bool WebdavDirectory::resource_exists(const std::string &resource_path) const {
    bool exists = true;
    try {
        m_request->HEAD(resource_path)->send();
    } catch (request::Response::NotFound &e) {
        exists = false;
    }
    return exists;
}
