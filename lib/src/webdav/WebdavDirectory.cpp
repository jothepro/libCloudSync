#include "WebdavDirectory.hpp"
#include "request/Request.hpp"
#include "request/Response.hpp"
#include "WebdavFile.hpp"
#include "WebdavExceptionTranslator.hpp"
#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"
#include <pugixml.hpp>
#include <filesystem>
#include <sstream>
#include <vector>

using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::webdav;
using namespace pugi;
namespace fs = std::filesystem;


const std::string WebdavDirectory::XML_QUERY =
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
    std::vector<std::shared_ptr<Resource>> resource_list;
    try {
        const auto response_xml = m_request->PROPFIND(m_base_url + m_dir_offset + m_path.generic_string())
                ->basic_auth(m_credentials->username(), m_credentials->password())
                ->header("Depth", "1")
                ->accept(Request::MIMETYPE_XML)
                ->content_type(Request::MIMETYPE_XML)
                ->body(XML_QUERY)->request().xml();
        resource_list = this->parse_xml_response(response_xml->root());
    } catch (...) {
        WebdavExceptionTranslator::translate(m_path);
    }
    return resource_list;
}

std::shared_ptr<Directory> WebdavDirectory::get_directory(const std::filesystem::path &path) const {
    const auto resource_path = append_path(path);
    std::shared_ptr<WebdavDirectory> directory;
    if (resource_path.generic_string() == "/") {
        // if it's the root we don't need to run a query
        directory = std::make_shared<WebdavDirectory>(m_base_url, "", "/",
                                                      m_credentials,
                                                      m_request, "");
    } else {
        try {
            const auto response_xml = m_request->PROPFIND(m_base_url + m_dir_offset + resource_path.generic_string())
                    ->basic_auth(m_credentials->username(), m_credentials->password())
                    ->header("Depth", "0")
                    ->accept(Request::MIMETYPE_XML)
                    ->content_type(Request::MIMETYPE_XML)
                    ->body(XML_QUERY)->request().xml();
            const auto resource_list = this->parse_xml_response(response_xml->root());
            if (resource_list.size() == 1) {
                directory = std::dynamic_pointer_cast<WebdavDirectory>(resource_list[0]);
                if(!directory) { // if the dynamic cast has failed, the returned resource is a file, not a directory
                    throw exceptions::resource::NoSuchResource(resource_path);
                }
            } else {
                throw exceptions::cloud::CommunicationError("cannot get resource description");
            }
        } catch (...) {
            WebdavExceptionTranslator::translate(resource_path);
        }
    }
    return directory;
}

void WebdavDirectory::remove() {
    if(m_path.generic_string() == "/") {
        throw exceptions::resource::PermissionDenied(m_path);
    } else {
        try {
            m_request->DELETE(m_base_url + m_dir_offset + m_path.generic_string())
                    ->basic_auth(m_credentials->username(), m_credentials->password())
                    ->request();
        } catch (...) {
            WebdavExceptionTranslator::translate(m_path);
        }
    }
}

std::shared_ptr<Directory> WebdavDirectory::create_directory(const std::filesystem::path &path) const {
    std::shared_ptr<WebdavDirectory> directory;
    const auto resource_path = append_path(path);
    create_parent_directories_if_missing(resource_path);
    try {
        m_request->MKCOL(m_base_url + m_dir_offset + resource_path.generic_string())
                ->basic_auth(m_credentials->username(), m_credentials->password())
                ->request();
        directory = std::make_shared<WebdavDirectory>(
                m_base_url,
                m_dir_offset,
                resource_path,
                m_credentials,
                m_request,
                resource_path.filename().generic_string());
    } catch(request::exceptions::response::MethodNotAllowed &e) {
        throw exceptions::resource::ResourceConflict(resource_path);
    } catch (...) {
        WebdavExceptionTranslator::translate(resource_path);
    }
    return directory;
}

std::shared_ptr<File> WebdavDirectory::create_file(const std::filesystem::path &path) const {
    const auto resource_path = append_path(path);
    std::shared_ptr<File> file;
    create_parent_directories_if_missing(resource_path);
    try {
        if(resource_exists(resource_path)) {
            throw exceptions::resource::ResourceConflict(resource_path);
        } else {
            const auto response = m_request->PUT(m_base_url + m_dir_offset + resource_path.generic_string())
                    ->basic_auth(m_credentials->username(), m_credentials->password())
                    ->content_type(Request::MIMETYPE_BINARY)->request();
            const auto revision = response.headers.at("etag");
            file = std::make_shared<WebdavFile>(
                    m_base_url + m_dir_offset,
                    resource_path,
                    m_credentials,
                    m_request,
                    resource_path.filename().generic_string(),
                    revision);
        }
    } catch (...) {
        WebdavExceptionTranslator::translate(resource_path);
    }
    return file;
}

std::shared_ptr<File> WebdavDirectory::get_file(const std::filesystem::path &path) const {
    const auto resource_path = append_path(path);
    std::shared_ptr<WebdavFile> file;
    try {
        const auto response_xml = m_request->PROPFIND(m_base_url + m_dir_offset + resource_path.generic_string())
                ->basic_auth(m_credentials->username(), m_credentials->password())
                ->header("Depth", "0")
                ->accept(Request::MIMETYPE_XML)
                ->content_type(Request::MIMETYPE_XML)
                ->body(XML_QUERY)->request().xml();
        const auto resourceList = this->parse_xml_response(response_xml->root());
        if (resourceList.size() == 1) {
            file = std::dynamic_pointer_cast<WebdavFile>(resourceList[0]);
            if(!file) { // if the dynamic cast has failed, the returned resource is a not a file
                throw exceptions::resource::NoSuchResource(resource_path);
            }
        } else {
            throw exceptions::cloud::CommunicationError("cannot get metadata for file");
        }
    } catch (...) {
        WebdavExceptionTranslator::translate(resource_path);
    }
    return file;
}

std::vector<std::shared_ptr<Resource>> WebdavDirectory::parse_xml_response(const xml_node &response) const {
    std::vector<std::shared_ptr<Resource>> resources;
    const auto responseNodeSets = response.select_nodes(
            "/*[local-name()='multistatus']/*[local-name()='response']");
    for (const auto responseNodeSet: responseNodeSets) {
        const auto responseNode = responseNodeSet.node();

        // read path from xml
        std::string resource_href = responseNode.select_node("./*[local-name()='href']").node().child_value();
        // remove path offset from the beginning of the path
        resource_href.erase(0, m_dir_offset.size());
        // remove any trailing slashes because webdav returns folders with
        // trailing slashes.
        resource_href = remove_trailing_slashes(resource_href);
        if (resource_href != m_path.generic_string()) {
            // parse the href as path so the filename/foldername can be
            // extracted
            const auto resource_path = fs::path(resource_href);
            std::shared_ptr<Resource> resource;
            // if the collection node exists, we can be sure this is a
            // directory, else it must be a file
            const auto filename = resource_path.filename().generic_string();
            if (responseNode.select_node(
                "./*[local-name()='propstat']"
                "/*[local-name()='prop']"
                "/*[local-name()='resourcetype']"
                "/*[local-name()='collection']")
            ) {
                resource = std::make_shared<WebdavDirectory>(
                        m_base_url,
                        m_dir_offset,
                        resource_href,
                        m_credentials,
                        m_request,
                        filename);
            } else if (const auto revision = responseNode.select_node(
                "./*[local-name()='propstat']"
                "/*[local-name()='prop']"
                "/*[local-name()='getetag']").node().child_value()
            ) {
                resource = std::make_shared<WebdavFile>(
                    m_base_url + m_dir_offset,
                    resource_href,
                    m_credentials,
                    m_request,
                    filename,
                    revision);
            }
            resources.push_back(resource);
        }
    }
    return resources;
}

bool WebdavDirectory::resource_exists(const std::filesystem::path &resource_path) const {
    bool exists = true;
    try {
        m_request->HEAD(m_base_url + m_dir_offset + resource_path.generic_string())
            ->basic_auth(m_credentials->username(), m_credentials->password())
            ->request();
    } catch (request::exceptions::response::NotFound &e) {
        exists = false;
    }
    return exists;
}

void WebdavDirectory::create_parent_directories_if_missing(const std::filesystem::path &absolute_path) const {
    const auto relative_path = absolute_path.lexically_relative(m_path);
    if(relative_path.has_parent_path()) {
        const auto relative_parent_path = relative_path.parent_path();
        if(relative_parent_path.filename().generic_string() != "..") {
            if(!resource_exists(m_path / relative_parent_path)) {
                create_directory(relative_parent_path);
            }
        }
    }
}
