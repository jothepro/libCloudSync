#include "box/BoxDirectory.hpp"
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

SCENARIO("BoxDirectory", "[directory][box]") {
    INIT_REQUEST();

    GIVEN("a box root directory") {
        const auto directory = std::make_shared<BoxDirectory>("0", "0", "/", request, "");

        THEN("the working dir should be '/'") {
            REQUIRE(directory->path() == "/");
        }

        AND_GIVEN("a GET request that returns a valid directory listing") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{
                    {"total_count", 2},
                    {"limit", 1000},
                    {"offset", 0},
                    {"order", {{{"by", "type"}, {"direction", "ASC"}}}},
                    {"entries",
                     {{{"id", "1234"}, {"etag", "1"}, {"type", "file"}, {"sequence_id", "3"}, {"name", "test.txt"}},
                      {{"id", "1235"},
                       {"etag", "42"},
                       {"type", "folder"},
                       {"sequence_id", "4"},
                       {"name", "testfolder"}}}}}
                    .dump(),
                "application/json"));

            WHEN("calling list_resources()") {
                auto list = directory->list_resources();
                THEN("the box items endpoint should be called with the id of the root folder (0)") {
                    Verify(Method((requestMock), request)).Once();
                    REQUIRE(requestRecording.front().verb == "GET");
                    REQUIRE(requestRecording.front().url == "https://api.box.com/2.0/folders/0/items");
                }

                THEN("a list of all resources contained in the directory should be returned") {
                    REQUIRE(list.size() == 2);
                    REQUIRE(list[0]->name() == "test.txt");
                    REQUIRE(list[0]->path() == "/test.txt");
                    REQUIRE(list[1]->name() == "testfolder");
                    REQUIRE(list[1]->path() == "/testfolder");
                }
            }

            WHEN("calling cd(testfolder), cd(testfolder/), cd(/testfolder/), get_directory(/subfolder/../testfolder)") {
                std::string path = GENERATE(
                    as<std::string>{},
                    "testfolder",
                    "testfolder/",
                    "/testfolder/",
                    "/subfolder/../testfolder");
                const auto newDir = directory->get_directory(path);

                THEN("the box items endpoint should be called with the id of the root folder (0)") {
                    Verify(Method((requestMock), request)).Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "https://api.box.com/2.0/folders/0/items");
                }

                THEN("the sub-folder called 'testfolder' should be returned") {
                    REQUIRE(newDir->name() == "testfolder");
                    REQUIRE(newDir->path() == "/testfolder");
                }
            }

            WHEN("calling get_file(test.txt)") {
                const auto file = directory->get_file("test.txt");
                THEN("the box items endpoint should be called with the id of the root folder (0)") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "https://api.box.com/2.0/folders/0/items");
                }

                THEN("the file called test.txt should be returned") {
                    REQUIRE(file->name() == "test.txt");
                    REQUIRE(file->path() == "/test.txt");
                }
            }
        }
        AND_GIVEN("a POST request that returns a new file description (in a list)") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{
                    {"total_count", 1},
                    {"entries",
                     {{
                         {"id", "12345"},
                         {"etag", "1234"},
                         {"type", "file"},
                         {"sequence_id", "3"},
                         {"name", "newfile.txt"}
                         // a lot more stuff comes here normally
                     }}}}
                    .dump(),
                "application/json"));
            WHEN("calling create_file(newfile.txt)") {
                directory->create_file("newfile.txt");

                THEN("the box upload endpoint should be called") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://upload.box.com/api/2.0/files/content");
                    REQUIRE_REQUEST(
                        0,
                        parameters.at(P::MIME_POSTFIELDS).at("attributes") ==
                            "{\"name\":\"newfile.txt\",\"parent\":{\"id\":\"0\"}}");
                    REQUIRE_REQUEST(0, parameters.at(P::MIME_POSTFILES).at("file") == "");
                }
            }
        }
        AND_GIVEN("another GET request that returns a valid directory listing twice") {
            WHEN_REQUEST()
                .RESPOND(request::Response(
                    200,
                    json{
                        {"total_count", 2},
                        {"limit", 1000},
                        {"offset", 0},
                        {"order", {{{"by", "type"}, {"direction", "ASC"}}}},
                        {"entries",
                         {{{"id", "1234"},
                           {"etag", "1"},
                           {"type", "file"},
                           {"sequence_id", "3"},
                           {"name", "testfile.txt"}},
                          {{"id", "1235"},
                           {"etag", "423"},
                           {"type", "folder"},
                           {"sequence_id", "4"},
                           {"name", "testfolder"}}}}}
                        .dump(),
                    "application/json"))
                .RESPOND(request::Response(
                    200,
                    json{
                        {"total_count", 1},
                        {"limit", 1000},
                        {"offset", 0},
                        {"order", {{{"by", "type"}, {"direction", "ASC"}}}},
                        {"entries",
                         {{{"id", "1236"},
                           {"etag", "423"},
                           {"type", "folder"},
                           {"sequence_id", "4"},
                           {"name", "testfolder2"}}}}}
                        .dump(),
                    "application/json"));
            WHEN("calling get_directory (testfolder/testfolder2)") {
                const auto newDir = directory->get_directory("testfolder/testfolder2");

                THEN("the box items endpoint should be called for '/' first, then for 'testfolder'") {
                    REQUIRE_REQUEST_CALLED().Twice();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "https://api.box.com/2.0/folders/0/items");
                    REQUIRE_REQUEST(1, verb == "GET");
                    REQUIRE_REQUEST(1, url == "https://api.box.com/2.0/folders/1235/items");
                }

                THEN("the sub-sub folder 'testfolder2' should be returned") {
                    REQUIRE(newDir->name() == "testfolder2");
                    REQUIRE(newDir->path() == "/testfolder/testfolder2");
                }
            }
        }

        AND_GIVEN("a POST request that returns a valid folder description") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{
                    {"id", "1234"},
                    {"etag", "1"},
                    {"type", "folder"},
                    {"sequence_id", "3"},
                    {"name", "newfolder"},
                }
                    .dump(),
                "application/json"));

            WHEN("calling create_directory(newfolder)") {
                const auto newFolder = directory->create_directory("newfolder");

                THEN("the box post-folders endpoint should be called with a correct json payload") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, url == "https://api.box.com/2.0/folders");
                    REQUIRE_REQUEST(0, body == "{\"name\":\"newfolder\",\"parent\":{\"id\":\"0\"}}");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Content-Type") == Request::MIMETYPE_JSON);
                }
                THEN("the new folder resource should be returned") {
                    REQUIRE(newFolder->name() == "newfolder");
                    REQUIRE(newFolder->path() == "/newfolder");
                }
            }
        }

        AND_GIVEN("a request that first returns a folder listing and then a folder description") {
            WHEN_REQUEST()
                .RESPOND(request::Response(
                    200,
                    json{
                        {"total_count", 1},
                        {"limit", 1000},
                        {"offset", 0},
                        {"order", {{{"by", "type"}, {"direction", "ASC"}}}},
                        {"entries",
                         {{{"id", "1236"},
                           {"etag", "423"},
                           {"type", "folder"},
                           {"sequence_id", "4"},
                           {"name", "testfolder"}}}}}
                        .dump(),
                    "application/json"))
                .RESPOND(request::Response(
                    200,
                    json{
                        {"id", "1234"},
                        {"etag", "1"},
                        {"type", "folder"},
                        {"sequence_id", "3"},
                        {"name", "newfolder"},
                    }
                        .dump(),
                    "application/json"));

            WHEN("calling create_directory (testfolder/newfolder)") {
                const auto newDir = directory->create_directory("testfolder/newfolder");

                THEN("the box items endpoint should be called on the root folder") {
                    REQUIRE_REQUEST_CALLED().Twice();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "https://api.box.com/2.0/folders/0/items");
                }
                THEN("the box endpoint to create a new folder should be called") {
                    REQUIRE_REQUEST_CALLED().Twice();
                    REQUIRE_REQUEST(1, verb == "POST");
                    REQUIRE_REQUEST(1, url == "https://api.box.com/2.0/folders");
                    REQUIRE_REQUEST(
                        1,
                        body == "{\"name\":\"newfolder\",\"parent\":{"
                                "\"id\":\"1236\"}}");
                    REQUIRE_REQUEST(1, parameters.at(P::HEADERS).at("Content-Type") == Request::MIMETYPE_JSON);
                }
                THEN("the newly created folder should be returned") {
                    REQUIRE(newDir->name() == "newfolder");
                    REQUIRE(newDir->path() == "/testfolder/newfolder");
                }
            }
        }
        WHEN("calling remove()") {
            THEN("a PermissionDenied Exception should be thrown") {
                REQUIRE_THROWS_AS(directory->remove(), CloudSync::Resource::PermissionDenied);
            }
        }
    }
    GIVEN("a box (non-root) directory") {
        const auto directory = std::make_shared<BoxDirectory>("1234", "0", "/somefolder", request, "somefolder");

        AND_GIVEN("a request that returns 204") {
            WHEN_REQUEST().RESPOND(request::Response(204));

            WHEN("calling remove()") {
                directory->remove();

                THEN("the box delete request should be called with the folder id") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "DELETE");
                    REQUIRE_REQUEST(0, url == "https://api.box.com/2.0/folders/1234");
                }
            }
        }

        AND_GIVEN("a GET request that returns the root folder description") {
            WHEN_REQUEST().RESPOND(request::Response(200, json{{"id", "0"}}.dump(), "application/json"));
            WHEN("calling get_directory(..)") {
                const auto root = directory->get_directory("..");
                THEN("the box folder endpoint should be called for the root folder (0)") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "https://api.box.com/2.0/folders/0");
                }

                THEN("the root directory should be returned") {
                    REQUIRE(root->name() == "");
                    REQUIRE(root->path() == "/");
                }
            }
        }
    }
}
