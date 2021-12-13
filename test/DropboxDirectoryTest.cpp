#include "dropbox/DropboxDirectory.hpp"
#include "CloudSync/Cloud.hpp"
#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"
#include "request/Request.hpp"
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

SCENARIO("DropboxDirectory", "[directory][dropbox]") {
    INIT_REQUEST();
    OAUTH_MOCK("mytoken");
    GIVEN("a dropbox root directory") {
        const auto directory = std::make_shared<DropboxDirectory>("/", credentials, request, "");
        THEN("the working dir should be '/'") {
            REQUIRE(directory->path() == "/");
        }
        WHEN("calling is_file()") {
            const bool is_file =directory->is_file();
            THEN("`true` should be returned") {
                REQUIRE(!is_file);
            }
        }
        AND_GIVEN("a request that returns a valid dropbox directory listing") {
            When(Method(requestMock, request)).Return(request::StringResponse(
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
                       {"content_hash", "a59943f6fe5e4cbc540"}}}},
                    {"cursor",
                     "AAEunngK5i6uSxwrSlvTngxpzli3qKoVouhB8LtojjN9gA"},
                    {"has_more", false}}
                    .dump(),
                "application/json"));

            WHEN("calling list_resources (list) on the directory") {
                auto list = directory->list_resources();

                THEN("the dropbox list_folder endpoint should be called with a json payload pointing to the root "
                     "folder") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/files/list_folder");
                    REQUIRE_REQUEST(0, body == "{\"path\":\"\",\"recursive\":false}");
                    REQUIRE_REQUEST(0, headers.at("Content-Type") == Request::MIMETYPE_JSON);
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
            When(Method(requestMock, request)).Return(request::StringResponse(
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
                .Return(request::StringResponse(
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
                .Return(request::StringResponse(
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
            WHEN("calling list_resources()") {
                auto list = directory->list_resources();
                THEN("a list of all resources contained in the 3 request responses should be returned") {
                    REQUIRE(list.size() == 3);
                    REQUIRE(list[0]->name() == "entry1");
                    REQUIRE(list[1]->name() == "entry2");
                    REQUIRE(list[2]->name() == "entry3");
                }
                THEN("the list_folder endpoint should have been called 3 times") {
                    Verify(Method(requestMock, request)).Exactly(3);
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/files/list_folder");
                    REQUIRE_REQUEST(0, headers.at("Content-Type") == Request::MIMETYPE_JSON);
                    REQUIRE_REQUEST(0, body == "{\"path\":\"\",\"recursive\":false}");

                    REQUIRE_REQUEST(1, verb == "POST");
                    REQUIRE_REQUEST(1, url == "https://api.dropboxapi.com/2/files/list_folder/continue");
                    REQUIRE_REQUEST(1, headers.at("Content-Type") == Request::MIMETYPE_JSON);
                    REQUIRE_REQUEST(1, body == "{\"cursor\":\"AAEunngK5i6uSxwrSlvTngxpzli3qKoVouhB8LtojjN9gA\"}");

                    REQUIRE_REQUEST(2, verb == "POST");
                    REQUIRE_REQUEST(2, url == "https://api.dropboxapi.com/2/files/list_folder/continue");
                    REQUIRE_REQUEST(2, headers.at("Content-Type") == Request::MIMETYPE_JSON);
                    REQUIRE_REQUEST(2, body == "{\"cursor\":\"BBEunngK5i6uSxwrSleliccAlcqKoVouhB8LtojjN9gB\"}");
                }
            }
        }
        AND_GIVEN("a request that returns a valid folder metadata description (with .tag)") {
            When(Method(requestMock, request)).Return(request::StringResponse(
                200,
                json{
                    {".tag", "folder"},
                    {"name", "test"},
                    {"path_lower", "/test"},
                    {"path_display", "/test"},
                    {"id", "id:O4T7biRqN_EAAAAAAAANRg"}}
                    .dump(),
                "application/json"));

            WHEN("calling cd(test), cd(test/), cd(/test/), get_directory(/test/more/..)") {
                std::string path = GENERATE(as<std::string>{}, "test", "test/", "/test/", "/test/more/..");
                auto newDir = directory->get_directory(path);
                THEN("the dropbox metadata endpoint should be called with a "
                     "json payload pointing to the desired folder") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/files/get_metadata");
                    REQUIRE_REQUEST(0, body == "{\"path\":\"/test\"}");
                    REQUIRE_REQUEST(0, headers.at("Content-Type") == Request::MIMETYPE_JSON);
                }
                THEN("a dir called 'test' should be returned") {
                    REQUIRE(newDir->name() == "test");
                    REQUIRE(newDir->path() == "/test");
                }
            }
            WHEN("calling get_file(test)") {
                THEN("a NoSuchFileOrDirectory Exception should be thrown because a folder description is returned") {
                    REQUIRE_THROWS_AS(directory->get_file("test"), CloudSync::exceptions::resource::NoSuchResource);
                }
            }
        }
        AND_GIVEN("a request that returns a valid folder metadata description (without .tag)") {
            When(Method(requestMock, request)).Return(request::StringResponse(
                200,
                json{{"metadata",
                      {{"name", "test"},
                       {"path_lower", "/test"},
                       {"path_display", "/test"},
                       {"id", "id:O4T7biRqN_EAAAAAAAANRg"}}}}
                    .dump(),
                "application/json"));

            WHEN("calling create_directory(test) on the current directory") {
                auto newDir = directory->create_directory("test");
                THEN("the dropbox create_folder_v2 endpoint should be called "
                     "with a json payload describing the new folder name") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/files/create_folder_v2");
                    REQUIRE_REQUEST(0, body == "{\"path\":\"/test\"}");
                    REQUIRE_REQUEST(0, headers.at("Content-Type") == Request::MIMETYPE_JSON);
                }
                THEN("a new dir called 'test' should be returned") {
                    REQUIRE(newDir->name() == "test");
                }
            }
        }
        AND_GIVEN("a request that returns a valid file metadata description (without .tag)") {
            When(Method(requestMock, request)).Return(request::StringResponse(
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

            WHEN("calling create_file(test.txt)") {
                auto newFile = directory->create_file("test.txt");

                THEN("the dropbox upload endpoint should be called with the arg param pointing to the new file and "
                     "with an empty request body") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://content.dropboxapi.com/2/files/upload");
                    REQUIRE_REQUEST(0, query_params.at("arg") == "{\"path\":\"/test.txt\"}");
                    REQUIRE_REQUEST(0, body.empty());
                    REQUIRE_REQUEST(0, headers.at("Content-Type") == Request::MIMETYPE_BINARY);
                }
                THEN("a new file called 'test.txt' should be returned") {
                    REQUIRE(newFile->name() == "test.txt");
                    REQUIRE(newFile->revision() == "0159d4da2a6fc2100000001a2504350");
                }
            }
        }
        AND_GIVEN("a request that returns a valid file metadata description (with .tag)") {
            When(Method(requestMock, request)).Return(request::StringResponse(
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

            WHEN("calling get_file(test.txt)") {
                auto file = directory->get_file("test.txt");
                THEN("the dropbox metadata endpoint should be called with a json payload pointing to the file") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/files/get_metadata");
                    REQUIRE_REQUEST(0, body == "{\"path\":\"/test.txt\"}");
                    REQUIRE_REQUEST(0, headers.at("Content-Type") == Request::MIMETYPE_JSON);
                }
                THEN("a file called 'test.txt' should be returned") {
                    REQUIRE(file->name() == "test.txt");
                    REQUIRE(file->revision() == "0159d4da2a6fc2100000001a2504350");
                }
            }
            WHEN("calling get_directory(test.txt)") {
                THEN("a NoSuchFileOrDirectory Exception should be thrown because a file description is returned") {
                    REQUIRE_THROWS_AS(directory->get_directory("test.txt"), CloudSync::exceptions::resource::NoSuchResource);
                }
            }
        }
        WHEN("deleting the current directory") {
            THEN("a PermissionDenied exception should be thrown because the root dir cannot be deleted") {
                REQUIRE_THROWS_AS(directory->remove(), CloudSync::exceptions::resource::PermissionDenied);
            }
        }
        AND_GIVEN("a request that returns an invalid (non-json) response") {
            When(Method(requestMock, request)).Return(request::StringResponse(200, "un-parseable", "text/plain"));
            WHEN("calling list_resources") {
                THEN("an InvalidResponse exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->list_resources(), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
            WHEN("calling get_directory(test)") {
                THEN("an InvalidResponse exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->get_directory("test"), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
            WHEN("calling create_directory(test)") {
                THEN("an InvalidResponse exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->create_directory("test"), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
            WHEN("calling create_file(test.txt)") {
                THEN("an InvalidResponse exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->create_file("test.txt"), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
            WHEN("calling get_file(test.txt)") {
                THEN("an InvalidResponse exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->get_file("test.txt"), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
        }
        AND_GIVEN("a request that fails with a 409 (Conflict) response because a resource could not be found") {
            When(Method(requestMock, request)).Throw(request::exceptions::response::Conflict(json{
                {"error_summary", "path/not_found/.."},
                {"error", {
                    {".tag", "path"},
                    {"path", {
                        {".tag", "not_found"}
                    }}
                }}
            }.dump()));
            WHEN("calling list_resources") {
                THEN("a NoSuchFileOrDirectory exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->list_resources(), CloudSync::exceptions::resource::NoSuchResource);
                }
            }
            WHEN("calling get_directory(test)") {
                THEN("a NoSuchFileOrDirectory exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->get_directory("test"), CloudSync::exceptions::resource::NoSuchResource);
                }
            }
            WHEN("calling get_file(test.txt)") {
                THEN("a NoSuchFileOrDirectory exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->get_file("test.txt"), CloudSync::exceptions::resource::NoSuchResource);
                }
            }
        }
        AND_GIVEN("a request that fails with a 409 (Conflict) that cannot be parsed") {
            When(Method(requestMock, request)).Throw(request::exceptions::response::Conflict("un-parseable"));
            WHEN("calling list_resources") {
                THEN("an InvalidResponse Exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->list_resources(), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
            WHEN("calling get_directory(test)") {
                THEN("an InvalidResponse Exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->get_directory("test"), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
            WHEN("calling get_file(test.txt)") {
                THEN("an InvalidResponse Exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->get_file("test.txt"), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
        }
    }
    GIVEN("a dropbox (non-root) directory") {
        const auto directory = std::make_shared<DropboxDirectory>("/test", credentials, request, "test");
        AND_GIVEN("a request that returns 200") {
            When(Method(requestMock, request)).Return(request::StringResponse(200));
            WHEN("deleting the current directory") {
                directory->remove();
                THEN("the dropbox delete_v2 endpoint should be called with a "
                     "json pointing to the to be deleted file") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/files/delete_v2");
                    REQUIRE_REQUEST(0, body == "{\"path\":\"/test\"}");
                    REQUIRE_REQUEST(0, headers.at("Content-Type") == Request::MIMETYPE_JSON);
                }
            }
        }
        AND_GIVEN("a request that fails with a 409 (Conflict) response because a path lookup has failed") {
            When(Method(requestMock, request)).Throw(request::exceptions::response::Conflict(json{
                {"error_summary", "path_lookup/not_found/.."},
                {"error", {{".tag", "path_lookup"}, {"path_lookup", {{".tag", "not_found"}}}}}}
                                                                 .dump()));
            WHEN("calling remove") {
                THEN("a NoSuchFileOrDirectory exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->remove(), CloudSync::exceptions::resource::NoSuchResource);
                }
            }
        }
        AND_GIVEN("a request that fails with a 409 (Conflict) that cannot be parsed") {
            When(Method(requestMock, request)).Throw(request::exceptions::response::Conflict("un-parseable"));
            WHEN("calling remove") {
                THEN("an InvalidResponse Exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->remove(), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
        }
    }
}
