#include "gdrive/GDriveFile.hpp"
#include "CloudSync/Cloud.hpp"
#include "request/Request.hpp"
#include "macros/request_mock.hpp"
#include "macros/shared_ptr_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <nlohmann/json.hpp>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;
using namespace CloudSync::gdrive;
using json = nlohmann::json;
using namespace CloudSync::request;
using P = request::Request::ParameterType;

SCENARIO("GDriveFile", "[file][gdrive]") {
    INIT_REQUEST();
    const std::string BASE_URL = "https://www.googleapis.com/drive/v3";

    GIVEN("a google drive file") {
        const auto file = std::make_shared<GDriveFile>(BASE_URL, "fileId", "/test.txt", request, "test.txt", "2");
        AND_GIVEN("a request that returns 204") {
            WHEN_REQUEST().RESPOND(request::Response(204));

            WHEN("calling rm()") {
                file->rm();
                THEN("the file endpoint should be called for deletion") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "DELETE");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/files/fileId");
                }
            }
        }
        AND_GIVEN("a request that returns a download link and a GET request that returns the file content") {
            WHEN_REQUEST()
                .RESPOND(request::Response(200, json{{"downloadUrl", "downloadlink"}}.dump(), "application/json"))
                .RESPOND(request::Response(200, "file content", "text/plain"));

            WHEN("calling read()") {
                const auto content = file->read();
                THEN("a request to get the download link and a request to download the actual content should be made") {
                    REQUIRE_REQUEST_CALLED().Twice();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/files/fileId");
                    REQUIRE_REQUEST(0, parameters.at(P::QUERY_PARAMS).at("fields") == "downloadUrl");
                    REQUIRE_REQUEST(1, verb == "GET");
                    REQUIRE_REQUEST(1, url == "downloadlink");
                }
                THEN("the files content should be returned") {
                    REQUIRE(content == "file content");
                }
            }
        }
        AND_GIVEN("a request that returns a new etag") {
            WHEN_REQUEST().RESPOND(request::Response(200, json{{"etag", "newetag"}}.dump(), "application/json"));

            WHEN("calling write(somenewcontent)") {
                file->write("somenewcontent");
                THEN("a PUT request should be made on the file upload endpoint") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "PUT");
                    REQUIRE_REQUEST(0, url == "https://www.googleapis.com/upload/drive/v2/files/fileId");
                    REQUIRE_REQUEST(0, parameters.at(P::QUERY_PARAMS).at("uploadType") == "media");
                    REQUIRE_REQUEST(0, parameters.at(P::QUERY_PARAMS).at("fields") == "etag");
                    REQUIRE_REQUEST(0, body == "somenewcontent");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("If-Match") == "2");
                }
                THEN("the file revision should be updated") {
                    REQUIRE(file->revision() == "newetag");
                }
            }
        }
        AND_GIVEN("a request that returns the current etag (no change)") {
            WHEN_REQUEST().RESPOND(request::Response(200, json{{"etag", "2"}}.dump(), "application/json"));

            WHEN("calling pollChange()") {
                bool hasChanged = file->pollChange();
                THEN("the etag field should be requested for the file") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "https://www.googleapis.com/drive/v3/files/fileId");
                    REQUIRE_REQUEST(0, parameters.at(P::QUERY_PARAMS).at("fields") == "etag");
                }
                THEN("false should be returned") {
                    REQUIRE(hasChanged == false);
                }
                THEN("the revision should be the same") {
                    REQUIRE(file->revision() == "2");
                }
            }
        }
        AND_GIVEN("a request that returns a new etag (new revision)") {
            WHEN_REQUEST().RESPOND(request::Response(200, json{{"etag", "thisisanewetag"}}.dump(), "application/json"));

            WHEN("calling pollChange()") {
                bool hasChanged = file->pollChange();
                THEN("true should be returned") {
                    REQUIRE(hasChanged == true);
                }
                THEN("the files revision should be updated") {
                    REQUIRE(file->revision() == "thisisanewetag");
                }
            }
        }
    }
}
