#include "dropbox/DropboxFile.hpp"
#include "request/Request.hpp"
#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "macros/request_mock.hpp"
#include "macros/oauth_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <nlohmann/json.hpp>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;
using namespace CloudSync::dropbox;
using json = nlohmann::json;
using namespace CloudSync::request;

SCENARIO("DropboxFile", "[file][dropbox]") {
    INIT_REQUEST();
    OAUTH_MOCK("mytoken");
    GIVEN("a DropboxFile instance") {
        const auto file = std::make_shared<DropboxFile>("/test.txt", credentials, request, "test.txt", "revision-id");
        AND_GIVEN("a request that returns 200") {
            When(Method(requestMock, request)).Return(request::StringResponse(200));
            WHEN("the file is deleted") {
                file->remove();
                THEN("the dropbox delete endpoint should be called with a json payload pointing to the file") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/files/delete_v2");
                    REQUIRE_REQUEST(0, body == "{\"path\":\"/test.txt\"}");
                    REQUIRE_REQUEST(0, headers.at("Content-Type") == Request::MIMETYPE_JSON);
                }
            }
        }
        AND_GIVEN("a request that returns binary data") {
            When(Method(requestMock, request_binary)).Return(request::BinaryResponse(200, {0x12, 0x13, 0x14}, "application/octet-stream"));

            WHEN("the file is being read") {
                const auto content = file->read_binary();
                THEN("the dropbox download endpoint should be called with an arg parameter pointing to the file") {
                    Verify(Method(requestMock, request_binary)).Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://content.dropboxapi.com/2/files/download");
                    REQUIRE_REQUEST(0, body == "");
                    REQUIRE_REQUEST(0, headers.at("Content-Type") == Request::MIMETYPE_TEXT);
                    REQUIRE_REQUEST(0, query_params.at("arg") == "{\"path\":\"/test.txt\"}");
                }
                THEN("the file-content should be returned") {
                    REQUIRE(content == std::vector<std::uint8_t>({0x12, 0x13, 0x14}));
                }
            }
        }
        AND_GIVEN("a request that returns an updated file metadata description") {
            When(Method(requestMock, request)).Return(request::StringResponse(
                200,
                json{
                    {"name", "test.txt"},
                    {"path_lower", "/test.txt"},
                    {"path_display", "/test.txt"},
                    {"id", "id:O4T7biRqN_EAAAAAAAANWg"},
                    {"client_modified", "2020-02-11T20:39:07Z"},
                    {"server_modified", "2020-02-11T20:39:07Z"},
                    {"rev", "newrevision"},
                    {"size", 5},
                    {"is_downloadable", true},
                    {"content_hash", "dcf37e3729a3e1063df9ebb284d642b7ae3a5fe80337bf25b5751d6a1d6bd97f"}}
                    .dump(),
                "application/json"));

            WHEN("writing to the file") {
                const std::string newContent = "awesome new content";
                file->write(newContent);
                THEN("the dropbox upload endpoint should have been called with an arg param pointing to the file and "
                     "requesting an update") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://content.dropboxapi.com/2/files/upload");
                    REQUIRE_REQUEST(0, body == newContent);
                    REQUIRE_REQUEST(0, headers.at("Content-Type") == Request::MIMETYPE_BINARY);
                    REQUIRE_REQUEST(
                        0,
                        query_params.at("arg") ==
                            "{\"mode\":{\".tag\":\"update\",\"update\":\"revision-id\"},\"path\":\"/test.txt\"}");
                }
                THEN("the revision of the file should have been updated") {
                    REQUIRE(file->revision() == "newrevision");
                }
            }
            WHEN("calling poll_change()") {
                const bool hasChanged = file->poll_change();
                THEN("the dropbox get_metadata endpoint should be called") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/files/get_metadata");
                    REQUIRE_REQUEST(0, body == "{\"path\":\"/test.txt\"}");
                    REQUIRE_REQUEST(0, headers.at("Content-Type") == Request::MIMETYPE_JSON);
                }
                THEN("true should be returned and the revision should be updated") {
                    REQUIRE(hasChanged);
                    REQUIRE(file->revision() == "newrevision");
                }
            }
        }
        AND_GIVEN("a request that returns 409 conflict") {
            When(Method(requestMock, request)).Throw(request::exceptions::response::Conflict());
            WHEN("writing to the file") {
                THEN("a ResourceHasChanged exception should be thrown") {
                    REQUIRE_THROWS_AS(file->write({"test"}), CloudSync::exceptions::resource::ResourceHasChanged);
                }
            }
        }
        AND_GIVEN("a request that returns a file description with the same revision") {
            When(Method(requestMock, request)).Return(request::StringResponse(200, json{{"rev", "revision-id"}}.dump(), "application/json"));
            WHEN("calling poll_change()") {
                const bool hasChanged = file->poll_change();
                THEN("false should be returned") {
                    REQUIRE(hasChanged == false);
                }
            }
        }
    }
}
