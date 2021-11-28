#include "onedrive/OneDriveFile.hpp"
#include "request/Request.hpp"
#include "macros/request_mock.hpp"
#include "macros/shared_ptr_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <nlohmann/json.hpp>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;
using namespace CloudSync::onedrive;
using json = nlohmann::json;
using namespace CloudSync::request;
using P = request::Request::ParameterType;

SCENARIO("OneDriveFile", "[file][onedrive]") {
    INIT_REQUEST();
    GIVEN("a OneDriveFile instance") {
        const auto file = std::make_shared<OneDriveFile>(
            "https://graph.microsoft.com/v1.0/me/drive/root",
            "/folder/file.txt",
            request,
            "file.txt",
            "file_revision");
        AND_GIVEN("a request that returns 204") {
            WHEN_REQUEST().RESPOND(request::Response(204));

            WHEN("calling remove()") {
                file->remove();
                THEN("the onedrive file endpoint should be called with DELETE") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "DELETE");
                    REQUIRE_REQUEST(0, url == "https://graph.microsoft.com/v1.0/me/drive/root:/folder/file.txt");
                }
            }
        }
        AND_GIVEN("a request that returns the files content") {
            WHEN_REQUEST().RESPOND(request::Response(200, "file content"));

            WHEN("calling read_as_string()") {
                const auto fileContent = file->read_as_string();

                THEN("the content get endpoint should be called with GET") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(
                        0,
                        url == "https://graph.microsoft.com/v1.0/me/"
                               "drive/root:/folder/file.txt:/content");
                }

                THEN("the file content should be returned") {
                    REQUIRE(fileContent == "file content");
                }
            }
        }

        AND_GIVEN("a PUT request that returns a new file item description (with new revision)") {
            WHEN_REQUEST().RESPOND(
                request::Response(200, json{{"eTag", "new_file_revision"}}.dump(), "application/json"));

            WHEN("calling write_string(new file content)") {
                file->write_string("new file content");
                THEN("the resource endpoint should be called with PUT & If-Match Header") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "PUT");
                    REQUIRE_REQUEST(
                        0,
                        url == "https://graph.microsoft.com/v1.0/me/"
                               "drive/root:/folder/file.txt:/content");
                    REQUIRE_REQUEST(0, body == "new file content");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("If-Match") == "file_revision");
                }

                THEN("the file revision should have been updated") {
                    REQUIRE(file->revision() == "new_file_revision");
                }
            }
        }

        AND_GIVEN("a request that throws a 412 Precondition Failed") {
            WHEN_REQUEST().Throw(request::Response::PreconditionFailed(
                json{{"error", {{"code", "notAllowed"}, {"message", "ETag does not match current item's value"}}}}
                    .dump()));

            WHEN("calling write_string(new content)") {
                THEN("a ResourceHasChanged Exception should be thrown") {
                    REQUIRE_THROWS_AS(file->write_string("new content"), Resource::ResourceHasChanged);
                }
            }
        }

        AND_GIVEN("a GET request that returns a file description (with new revision)") {
            WHEN_REQUEST().RESPOND(
                request::Response(200, json{{"eTag", "new_file_revision"}}.dump(), "application/json"));

            WHEN("calling poll_change()") {
                bool fileChanged = file->poll_change();
                THEN("the file item endpoint should be called") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(
                        0,
                        url == "https://graph.microsoft.com/v1.0/me/"
                               "drive/root:/folder/file.txt");
                }
                THEN("the result should be true (the file has changed)") {
                    REQUIRE(fileChanged == true);
                }
            }
        }
        AND_GIVEN("a GET request that returns a file description (with the same old revision)") {
            WHEN_REQUEST().RESPOND(request::Response(200, json{{"eTag", "file_revision"}}.dump(), "application/json"));

            WHEN("calling poll_change()") {
                bool fileChanged = file->poll_change();
                THEN("the file item endpoint should be called") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(
                        0,
                        url == "https://graph.microsoft.com/v1.0/me/"
                               "drive/root:/folder/file.txt");
                }
                THEN("the resutl should be false (the file has not changed)") {
                    REQUIRE(fileChanged == false);
                }
            }
        }
    }
}
