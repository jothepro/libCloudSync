#include "box/BoxFile.hpp"
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
using namespace CloudSync::box;
using json = nlohmann::json;
using namespace CloudSync::request;
using P = request::Request::ParameterType;

SCENARIO("BoxFile", "[file][box]") {
    INIT_REQUEST();
    GIVEN("a box file instance") {
        const auto file = std::make_shared<BoxFile>("1234", "/folder/filename.txt", request, "filename.txt", "abcd");

        AND_GIVEN("a DELETE request that returns 204") {
            WHEN_REQUEST().RESPOND(request::Response(204));

            WHEN("calling rm()") {
                file->rm();
                THEN("the box file-delete endpoint should be called") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "DELETE");
                    REQUIRE_REQUEST(0, url == "https://api.box.com/2.0/files/1234");
                }
            }
        }

        AND_GIVEN("a request that returns 404") {
            WHEN_REQUEST().Throw(request::Response::NotFound(""));

            WHEN("calling rm()") {
                THEN("a NoSuchFileOrDirectory Exception should be thrown") {
                    REQUIRE_THROWS_AS(file->rm(), CloudSync::Resource::NoSuchFileOrDirectory);
                }
            }
        }

        AND_GIVEN("a request that returns the file content") {
            WHEN_REQUEST().RESPOND(request::Response(200, "filecontent", "text/plain"));
            WHEN("calling read()") {
                const auto fileContent = file->read();

                THEN("the box download endpoint should be called") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "https://api.box.com/2.0/files/1234/content");
                }

                THEN("the file-content should be returned") {
                    REQUIRE(fileContent == "filecontent");
                }
            }
        }

        AND_GIVEN("a POST request that returns a valid file description") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{
                    {"total_count", 1},
                    {"entries",
                     {{
                         {"id", "1234"},
                         {"etag", "newetag"},
                         // a lot more stuff that doesn't matter here
                     }}}}
                    .dump(),
                "application/json"));

            WHEN("calling write(newcontent)") {
                file->write("newcontent");

                THEN("the box file update endpoint should be called") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://upload.box.com/api/2.0/files/1234/content");
                    REQUIRE_REQUEST(0, parameters.at(P::MIME_POSTFIELDS).at("attributes") == "{}");
                    REQUIRE_REQUEST(0, parameters.at(P::MIME_POSTFILES).at("file") == "newcontent");
                }

                THEN("the revision should be updated") {
                    REQUIRE(file->revision() == "newetag");
                }
            }
        }
        AND_GIVEN("a request that returns a changed file description") {
            WHEN_REQUEST().RESPOND(request::Response(200, json{{"etag", "newetag"}}.dump(), "application/json"));

            WHEN("calling pollChange()") {
                const bool hasChanged = file->pollChange();
                THEN("a GET request should be made to ask for the file metadata") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "https://api.box.com/2.0/files/1234");
                }
                THEN("true should be returned and the new revision should be set") {
                    REQUIRE(hasChanged);
                    REQUIRE(file->revision() == "newetag");
                }
            }
        }
        AND_GIVEN("a POST request that returns the same file revision") {
            WHEN_REQUEST().RESPOND(request::Response(200, json{{"etag", "abcd"}}.dump(), "application/json"));
            WHEN("calling pollChange()") {
                const bool hasChanged = file->pollChange();
                THEN("false should be returned") {
                    REQUIRE(hasChanged == false);
                }
            }
        }
    }
}