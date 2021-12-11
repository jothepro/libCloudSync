#include "Request.hpp"
#include <chrono>

using namespace std::chrono;

namespace CloudSync::request {

    const std::string Request::MIMETYPE_XML = "application/xml";
    const std::string Request::MIMETYPE_JSON = "application/json";
    const std::string Request::MIMETYPE_BINARY = "application/octet-stream";
    const std::string Request::MIMETYPE_TEXT = "text/plain";

    void Request::set_follow_redirects(bool follow) {
        this->m_option_follow_redirects = follow;
    }

    void Request::set_verbose(bool verbose) {
        this->m_option_verbose = verbose;
    }

    std::shared_ptr<Request> Request::json_body(const nlohmann::json &json_data) {
        content_type(Request::MIMETYPE_JSON);
        return body(json_data.dump());
    }

    std::shared_ptr<Request> Request::accept(const std::string &mimetype) {
        return header("Accept", mimetype);
    }

    std::shared_ptr<Request> Request::content_type(const std::string &mimetype) {
        return header("Content-Type", mimetype);
    }

    std::shared_ptr<Request> Request::if_match(const std::string &etag) {
        return header("If-Match", etag);
    }

} // namespace CloudSync::request
