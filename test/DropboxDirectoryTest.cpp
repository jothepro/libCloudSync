#include "dropbox/DropboxDirectory.hpp"
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
using namespace CloudSync::dropbox;
using json = nlohmann::json;
using namespace CloudSync::request;
using P = request::Request::ParameterType;

SCENARIO("DropboxDirectory", "[directory][dropbox]") {
    INIT_REQUEST();

    GIVEN("a dropbox root directory") {

        const auto directory = std::make_shared<DropboxDirectory>("/", request, "");
        THEN("the working dir should be '/'") {
            REQUIRE(directory->pwd() == "/");
        }

        AND_GIVEN("a request that returns a valid dropbox directory listing") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{
                    {"entries",
                     {{{".tag", "folder"},
                       {"name", "test"},
                       {"path_lower", "/test"},
                       {"path_display", "/test"},
                       {"id", "id:O4T7biRqN_EAAAAAAAANRg"}},
                      {{".tag", "file"},
                       {"name", "test.txt"},
                       {"path_lower", "/test.txt"},
                       {"path_display", "/test.txt"},
                       {"id", "id:O4T7biRqN_EAAAAAAAANR"},
                       {"client_modified", "2020-01-29T21:00:50Z"},
                       {"server_modified", "2020-01-29T21:00:50Z"},
                       {"rev", "0159d4da2a6fc2100000001a2504350"},
                       {"size", 11048},
                       {"is_downloadable", true},
                       {"content_hash",
                        "a59943f6fe5e4cbc25366a1c412b4278bc74984353265c2160"
                        "607a073c9cb540"}}}},
                    {"cursor",
                     "AAEunngK5i6uSxwrSlvTngxpzli3qKoVouhB8LtojjN9gA"},
                    {"has_more", false}}
                    .dump(),
                "application/json"));

            WHEN("calling ls (list) on the directory") {
                auto list = directory->ls();

                THEN("the dropbox list_folder endpoint should be called with a json payload pointing to the root "
                     "folder") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/files/list_folder");
                    REQUIRE_REQUEST(0, body == "{\"path\":\"\",\"recursive\":false}");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Content-Type") == Request::MIMETYPE_JSON);
                }
                THEN("a list of all resources contained in the directory should be returned") {
                    REQUIRE(list.size() == 2);
                    REQUIRE(list[0]->name() == "test");
                    REQUIRE(list[0]->path() == "/test");
                    REQUIRE(list[1]->name() == "test.txt");
                    REQUIRE(list[1]->path() == "/test.txt");
                }
            }
        }
        AND_GIVEN("2 requests that return a valid dropbox directory listing with `has_more: true` and 1 returning `has_more: false`") {
            WHEN_REQUEST()
                .RESPOND(request::Response(
                    200,
                    json{
                        {"entries",
                            {
                                {
                                    {".tag", "folder"},
                                    {"name", "entry1"},
                                    {"path_lower", "/entry1"},
                                    {"path_display", "/entry1"},
                                    {"id", "id:O4T7biRqN_EAAAAAAAANRg"}
                                }
                            }
                        },
                        {"cursor", "AAEunngK5i6uSxwrSlvTngxpzli3qKoVouhB8LtojjN9gA"},
                        {"has_more", true}
                    }.dump(),
                    "application/json"))
                .RESPOND(request::Response(
                    200,
                    json{
                        {"entries",
                            {
                                {
                                    {".tag", "folder"},
                                    {"name", "entry2"},
                                    {"path_lower", "/entry2"},
                                    {"path_display", "/entry2"},
                                    {"id", "id:14T7biRqN_EniaesuAAANR6"}
                                }
                            }
                        },
                        {"cursor", "BBEunngK5i6uSxwrSleliccAlcqKoVouhB8LtojjN9gB"},
                        {"has_more", true}
                    }.dump(),
                    "application/json"))
                .RESPOND(request::Response(
                    200,
                    json{
                        {"entries",
                            {
                                {
                                    {".tag", "folder"},
                                    {"name", "entry3"},
                                    {"path_lower", "/entry3"},
                                    {"path_display", "/entry3"},
                                    {"id", "id:24T7bi5aeoWNEniaesuAAANRf"}
                                }
                            }
                        },
                        {"cursor", "CCEunngK5Wos5PwrSleliccAlcqKoVouhoeitojjN9gc"},
                        {"has_more", false}
                    }.dump(),
                    "application/json"));
            WHEN("calling ls()") {
                auto list = directory->ls();
                THEN("a list of all resources contained in the 3 request responses should be returned") {
                    REQUIRE(list.size() == 3);
                    REQUIRE(list[0]->name() == "entry1");
                    REQUIRE(list[1]->name() == "entry2");
                    REQUIRE(list[2]->name() == "entry3");
                }
                THEN("the list_folder endpoint should have been called 3 times") {
                    REQUIRE_REQUEST_CALLED().Exactly(3);
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/files/list_folder");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Content-Type") == Request::MIMETYPE_JSON);
                    REQUIRE_REQUEST(0, body == "{\"path\":\"\",\"recursive\":false}");

                    REQUIRE_REQUEST(1, verb == "POST");
                    REQUIRE_REQUEST(1, url == "https://api.dropboxapi.com/2/files/list_folder/continue");
                    REQUIRE_REQUEST(1, parameters.at(P::HEADERS).at("Content-Type") == Request::MIMETYPE_JSON);
                    REQUIRE_REQUEST(1, body == "{\"cursor\":\"AAEunngK5i6uSxwrSlvTngxpzli3qKoVouhB8LtojjN9gA\"}");

                    REQUIRE_REQUEST(2, verb == "POST");
                    REQUIRE_REQUEST(2, url == "https://api.dropboxapi.com/2/files/list_folder/continue");
                    REQUIRE_REQUEST(2, parameters.at(P::HEADERS).at("Content-Type") == Request::MIMETYPE_JSON);
                    REQUIRE_REQUEST(2, body == "{\"cursor\":\"BBEunngK5i6uSxwrSleliccAlcqKoVouhB8LtojjN9gB\"}");
                }
            }
        }
        AND_GIVEN("a POST request that returns a valid folder metadata description (with .tag)") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{
                    {".tag", "folder"},
                    {"name", "test"},
                    {"path_lower", "/test"},
                    {"path_display", "/test"},
                    {"id", "id:O4T7biRqN_EAAAAAAAANRg"}}
                    .dump(),
                "application/json"));

            WHEN("calling cd(test), cd(test/), cd(/test/), cd(/test/more/..)") {
                std::string path = GENERATE(as<std::string>{}, "test", "test/", "/test/", "/test/more/..");
                auto newDir = directory->cd(path);
                THEN("the dropbox metadata endpoint should be called with a "
                     "json payload pointing to the desired folder") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/files/get_metadata");
                    REQUIRE_REQUEST(0, body == "{\"path\":\"/test\"}");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Content-Type") == Request::MIMETYPE_JSON);
                }
                THEN("a dir called 'test' should be returned") {
                    REQUIRE(newDir->name() == "test");
                    REQUIRE(newDir->pwd() == "/test");
                }
            }
            WHEN("calling file(test)") {
                THEN("a NoSuchFileOrDirectory Exception should be thrown because a folder description is returned") {
                    REQUIRE_THROWS_AS(directory->file("test"), CloudSync::Resource::NoSuchFileOrDirectory);
                }
            }
        }
        AND_GIVEN("a POST request that returns a valid folder metadata description (without .tag)") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{{"metadata",
                      {{"name", "test"},
                       {"path_lower", "/test"},
                       {"path_display", "/test"},
                       {"id", "id:O4T7biRqN_EAAAAAAAANRg"}}}}
                    .dump(),
                "application/json"));

            WHEN("calling mkdir(test) on the current directory") {
                auto newDir = directory->mkdir("test");
                THEN("the dropbox create_folder_v2 endpoint should be called "
                     "with a json payload describing the new folder name") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/files/create_folder_v2");
                    REQUIRE_REQUEST(0, body == "{\"path\":\"/test\"}");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Content-Type") == Request::MIMETYPE_JSON);
                }
                THEN("a new dir called 'test' should be returned") {
                    REQUIRE(newDir->name() == "test");
                }
            }
        }
        AND_GIVEN("a POST request that returns a valid file metadata description (without .tag)") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{
                    {"name", "test.txt"},
                    {"path_lower", "/test.txt"},
                    {"path_display", "/test.txt"},
                    {"id", "id:O4T7biRqN_EAAAAAAAANR"},
                    {"client_modified", "2020-01-29T21:00:50Z"},
                    {"server_modified", "2020-01-29T21:00:50Z"},
                    {"rev", "0159d4da2a6fc2100000001a2504350"},
                    {"size", 11048},
                    {"is_downloadable", true},
                    {"content_hash", "a59943f6fe5e4cbc25366a1c412b4278bc74984353265c2160607a073c9cb540"}}
                    .dump(),
                "application/json"));

            WHEN("calling touch(test.txt)") {
                auto newFile = directory->touch("test.txt");

                THEN("the dropbox upload endpoint should be called with the arg param pointing to the new file and "
                     "with an empty request body") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://content.dropboxapi.com/2/files/upload");
                    REQUIRE_REQUEST(0, parameters.at(P::QUERY_PARAMS).at("arg") == "{\"path\":\"/test.txt\"}");
                    REQUIRE_REQUEST(0, body.empty());
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Content-Type") == Request::MIMETYPE_BINARY);
                }
                THEN("a new file called 'test.txt' should be returned") {
                    REQUIRE(newFile->name() == "test.txt");
                    REQUIRE(newFile->revision() == "0159d4da2a6fc2100000001a2504350");
                }
            }
        }
        AND_GIVEN("a POST request that returns a valid file metadata description (with .tag)") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{
                    {".tag", "file"},
                    {"name", "test.txt"},
                    {"path_lower", "/test.txt"},
                    {"path_display", "/test.txt"},
                    {"id", "id:O4T7biRqN_EAAAAAAAANR"},
                    {"client_modified", "2020-01-29T21:00:50Z"},
                    {"server_modified", "2020-01-29T21:00:50Z"},
                    {"rev", "0159d4da2a6fc2100000001a2504350"},
                    {"size", 11048},
                    {"is_downloadable", true},
                    {"content_hash", "a59943f6fe5e4cbc25366a1c412b4278bc74984353265c2160607a073c9cb540"}}
                    .dump(),
                "application/json"));

            WHEN("calling file(test.txt)") {
                auto file = directory->file("test.txt");
                THEN("the dropbox metadata endpoint should be called with a json payload pointing to the file") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/files/get_metadata");
                    REQUIRE_REQUEST(0, body == "{\"path\":\"/test.txt\"}");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Content-Type") == Request::MIMETYPE_JSON);
                }
                THEN("a file called 'test.txt' should be returned") {
                    REQUIRE(file->name() == "test.txt");
                    REQUIRE(file->revision() == "0159d4da2a6fc2100000001a2504350");
                }
            }
            WHEN("calling cd(test.txt)") {
                THEN("a NoSuchFileOrDirectory Exception should be thrown because a file description is returned") {
                    REQUIRE_THROWS_AS(directory->cd("test.txt"), CloudSync::Resource::NoSuchFileOrDirectory);
                }
            }
        }
        WHEN("deleting the current directory") {
            THEN("a PermissionDenied exception should be thrown because the root dir cannot be deleted") {
                REQUIRE_THROWS_AS(directory->rmdir(), CloudSync::Resource::PermissionDenied);
            }
        }
        AND_GIVEN("a POST request that returns an invalid (non-json) response") {
            WHEN_REQUEST().RESPOND(request::Response(200, "un-parseable", "text/plain"));
            WHEN("calling ls") {
                THEN("an InvalidResponse exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->ls(), CloudSync::Cloud::InvalidResponse);
                }
            }
            WHEN("calling cd(test)") {
                THEN("an InvalidResponse exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->cd("test"), CloudSync::Cloud::InvalidResponse);
                }
            }
            WHEN("calling mkdir(test)") {
                THEN("an InvalidResponse exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->mkdir("test"), CloudSync::Cloud::InvalidResponse);
                }
            }
            WHEN("calling touch(test.txt)") {
                THEN("an InvalidResponse exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->touch("test.txt"), CloudSync::Cloud::InvalidResponse);
                }
            }
            WHEN("calling file(test.txt)") {
                THEN("an InvalidResponse exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->file("test.txt"), CloudSync::Cloud::InvalidResponse);
                }
            }
        }
        AND_GIVEN("a POST request that fails with a 409 (Conflict) response because a resource could not be found") {
            WHEN_REQUEST().Throw(request::Response::Conflict(json{
                {"error_summary", "path/not_found/.."},
                {"error", {{".tag", "path"}, {"path", {{".tag", "not_found"}}}}}}
                                                                 .dump()));
            WHEN("calling ls") {
                THEN("a NoSuchFileOrDirectory exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->ls(), CloudSync::Resource::NoSuchFileOrDirectory);
                }
            }
            WHEN("calling cd(test)") {
                THEN("a NoSuchFileOrDirectory exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->cd("test"), CloudSync::Resource::NoSuchFileOrDirectory);
                }
            }
            WHEN("calling file(test.txt)") {
                THEN("a NoSuchFileOrDirectory exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->file("test.txt"), CloudSync::Resource::NoSuchFileOrDirectory);
                }
            }
        }
        AND_GIVEN("a POST request that fails with a 409 (Conflict) that cannot be parsed") {
            WHEN_REQUEST().Throw(request::Response::Conflict("un-parseable"));
            WHEN("calling ls") {
                THEN("an InvalidResponse Exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->ls(), CloudSync::Cloud::InvalidResponse);
                }
            }
            WHEN("calling cd(test)") {
                THEN("an InvalidResponse Exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->cd("test"), CloudSync::Cloud::InvalidResponse);
                }
            }
            WHEN("calling file(test.txt)") {
                THEN("an InvalidResponse Exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->file("test.txt"), CloudSync::Cloud::InvalidResponse);
                }
            }
        }
    }
    GIVEN("a dropbox (non-root) directory") {
        const auto directory = std::make_shared<DropboxDirectory>("/test", request, "test");
        AND_GIVEN("a POST request that returns 200") {
            WHEN_REQUEST().RESPOND(request::Response(200, "", ""));
            WHEN("deleting the current directory") {
                directory->rmdir();
                THEN("the dropbox delete_v2 endpoint should be called with a "
                     "json pointing to the to be deleted file") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/files/delete_v2");
                    REQUIRE_REQUEST(0, body == "{\"path\":\"/test\"}");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Content-Type") == Request::MIMETYPE_JSON);
                }
            }
        }
        AND_GIVEN("a request that fails with a 409 (Conflict) response because a path lookup has failed") {
            WHEN_REQUEST().Throw(request::Response::Conflict(json{
                {"error_summary", "path_lookup/not_found/.."},
                {"error", {{".tag", "path_lookup"}, {"path_lookup", {{".tag", "not_found"}}}}}}
                                                                 .dump()));
            WHEN("calling rmdir") {
                THEN("a NoSuchFileOrDirectory exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->rmdir(), CloudSync::Resource::NoSuchFileOrDirectory);
                }
            }
        }
        AND_GIVEN("a request that fails with a 409 (Conflict) that cannot be parsed") {
            WHEN_REQUEST().Throw(request::Response::Conflict("un-parseable"));
            WHEN("calling rmdir") {
                THEN("an InvalidResponse Exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->rmdir(), CloudSync::Cloud::InvalidResponse);
                }
            }
        }
    }
}
