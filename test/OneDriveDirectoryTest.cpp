#include "onedrive/OneDriveDirectory.hpp"
#include "CloudSync/Cloud.hpp"
#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "request/Request.hpp"
#include "macros/request_mock.hpp"
#include "macros/oauth_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <nlohmann/json.hpp>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;
using namespace CloudSync::onedrive;
using json = nlohmann::json;
using namespace CloudSync::request;

SCENARIO("OneDriveDirectory", "[directory][onedrive]") {
    INIT_REQUEST();
    OAUTH_MOCK("mytoken");
    GIVEN("a onedrive root directory") {
        const auto directory =
            std::make_shared<OneDriveDirectory>("https://graph.microsoft.com/v1.0/me/drive/root", "/", credentials, request, "");

        AND_GIVEN("a request that returns a valid directory listing") {
            When(Method(requestMock, request)).Return(request::StringResponse(
                200,
                json{{"value",
                      {{{"id", "ABW1234"},
                        {"@microsoft.graph.downloadUrl",
                         "https://public.ch.files.1drv.com/"
                         "filedownloadlink"},
                        {"eTag", "somerevision"},
                        {"name", "somefile.txt"},
                        {"parentReference", {{"path", "/drive/root:"}}},
                        {"file", {{"mimeType", "text/plain"}}}},
                       {{"name", "somefolder"},
                        {"parentReference", {{"path", "/drive/root:"}}},
                        {"folder", {{"childCount", 0}}}}}}}
                    .dump(),
                "application/json"));

            WHEN("calling list_resources") {
                const auto dirList = directory->list_resources();

                THEN("the /children graph endpoint should have been called") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "https://graph.microsoft.com/v1.0/me/drive/root/children");
                }

                THEN("a list with one file & one directory should be returned") {
                    REQUIRE(dirList.size() == 2);
                    REQUIRE(dirList[0]->name() == "somefile.txt");
                    REQUIRE(dirList[0]->path() == "/somefile.txt");
                    REQUIRE(dirList[1]->name() == "somefolder");
                    REQUIRE(dirList[1]->path() == "/somefolder");
                }
            }
        }
        AND_GIVEN("a request that returns a valid folder description") {
            When(Method(requestMock, request)).Return(request::StringResponse(
                200,
                json{
                    {"name", "somefolder"},
                    {"parentReference", {{"path", "/drive/root:"}}},
                    {"folder", {{"childCount", 0}}}}
                    .dump(),
                "application/json"));

            WHEN("calling get_directory(somefolder), get_directory(/somefolder), get_directory(/somefolder/), get_directory(/somefolder/some/..)") {
                std::string path =
                    GENERATE(as<std::string>{}, "somefolder", "/somefolder", "/somefolder/", "/somefolder/some/..");
                const auto newDirectory = directory->get_directory(path);
                THEN("the graph resource endpoint should have been called") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "https://graph.microsoft.com/v1.0/me/drive/root:/somefolder");
                }
                THEN("the folder 'somefolder' should be returned") {
                    REQUIRE(newDirectory->name() == "somefolder");
                    REQUIRE(newDirectory->path() == "/somefolder");
                }
            }
        }
    }
    GIVEN("a onedrive directory (non root)") {
        const auto directory = std::make_shared<OneDriveDirectory>(
            "https://graph.microsoft.com/v1.0/me/drive/root",
            "/some/folder",
            credentials,
            request,
            "folder");
        AND_GIVEN("a request that returns a directory listing") {
            When(Method(requestMock, request)).Return(request::StringResponse(
                200,
                json{{"value",
                      {{{"id", "ABW1234"},
                        {"@microsoft.graph.downloadUrl",
                         "https://public.ch.files.1drv.com/"
                         "filedownloadlink"},
                        {"eTag", "somerevision"},
                        {"name", "somefile.txt"},
                        {"parentReference", {{"path", "/drive/root:/some/folder"}}},
                        {"file", {{"mimeType", "text/plain"}}}},
                       {{"name", "somefolder"},
                        {"parentReference", {{"path", "/drive/root:/some/folder"}}},
                        {"folder", {{"childCount", 0}}}}}}}
                    .dump(),
                "application/json"));

            WHEN("calling list_resources()") {
                const auto dirList = directory->list_resources();

                THEN("the /childern grap endpoint should be called on the resource") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "https://graph.microsoft.com/v1.0/me/drive/root:/some/folder:/children");
                }

                THEN("a list with one file & one directory should be returned") {
                    REQUIRE(dirList.size() == 2);
                    REQUIRE(dirList[0]->name() == "somefile.txt");
                    REQUIRE(dirList[0]->path() == "/some/folder/somefile.txt");
                    REQUIRE(dirList[1]->name() == "somefolder");
                    REQUIRE(dirList[1]->path() == "/some/folder/somefolder");
                }
            }
        }

        AND_GIVEN("a request that throws 404 Resource Not Found") {
            When(Method(requestMock, request)).Throw(request::exceptions::response::NotFound(
                json{{"error", {{"code", "itemNotFound"}, {"message", "The resource could not be found."}}}}.dump()));

            WHEN("calling list_resources()") {
                THEN("a NoSuchFileOrDirectory Exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->list_resources(), CloudSync::exceptions::resource::NoSuchResource);
                }
            }
        }

        AND_GIVEN("a request that returns a valid folder description") {
            When(Method(requestMock, request)).Return(request::StringResponse(
                200,
                json{
                    {"name", "somefolder"},
                    {"parentReference", {{"path", "/drive/root:/some/folder"}}},
                    {"folder", {{"childCount", 0}}}}
                    .dump(),
                "application/json"));

            WHEN("calling get_directory(somefolder)") {
                const auto newDirectory = directory->get_directory("somefolder");
                THEN("the graph resource endpoint should have been called") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "https://graph.microsoft.com/v1.0/me/drive/root:/some/folder/somefolder");
                }
                THEN("the folder 'somefolder' should be returned") {
                    REQUIRE(newDirectory->name() == "somefolder");
                    REQUIRE(newDirectory->path() == "/some/folder/somefolder");
                }
            }
        }

        AND_GIVEN("a request that returns a valid file description") {
            When(Method(requestMock, request)).Return(request::StringResponse(
                200,
                json{
                    {"id", "ABW1234"},
                    {"eTag", "somerevision"},
                    {"name", "somefile.txt"},
                    {"parentReference", {{"path", "/drive/root:/some/folder"}}},
                    {"file", {{"mimeType", "text/plain"}}}}
                    .dump(),
                "application/json"));

            WHEN("calling get_file(somefile.txt)") {
                const auto file = directory->get_file("somefile.txt");

                THEN("the item endpoint should be called on the file path") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(
                        0,
                        url == "https://graph.microsoft.com/v1.0/me/drive/root:/some/folder/somefile.txt");
                }

                THEN("the requested file would be returned") {
                    REQUIRE(file->name() == "somefile.txt");
                    REQUIRE(file->path() == "/some/folder/somefile.txt");
                    REQUIRE(file->revision() == "somerevision");
                }
            }
        }

        AND_GIVEN("a request that throws 404 Resource Not Found") {
            When(Method(requestMock, request)).Throw(request::exceptions::response::NotFound());

            WHEN("calling get_file(somefile.txt)") {
                THEN("a NoSuchFileOrDirectory Exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->get_file("somefile.txt"), CloudSync::exceptions::resource::NoSuchResource);
                }
            }
        }

        AND_GIVEN("a request that returns a valid file description") {
            When(Method(requestMock, request)).Return(request::StringResponse(
                200,
                json{
                    {"id", "ABW1234"},
                    {"eTag", "somerevision"},
                    {"name", "somefile.txt"},
                    {"parentReference", {{"path", "/drive/root:/some/folder"}}},
                    {"file", {{"mimeType", "text/plain"}}}}
                    .dump(),
                "application/json"));

            WHEN("calling create_file(somefile.txt)") {
                const auto newFile = directory->create_file("somefile.txt");
                THEN("the content endpoint should be called with no file content on the desired resource path") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "PUT");
                    REQUIRE_REQUEST(
                        0,
                        url == "https://graph.microsoft.com/v1.0/me/drive/root:"
                               "/some/folder/somefile.txt:/content");
                    REQUIRE_REQUEST(0, body == "");
                }
                THEN("the new file should be returned") {
                    REQUIRE(newFile->name() == "somefile.txt");
                    REQUIRE(newFile->path() == "/some/folder/somefile.txt");
                    REQUIRE(newFile->revision() == "somerevision");
                }
            }
        }

        AND_GIVEN("a request sequence that returns 200 and a valid folder description") {
            When(Method(requestMock, request)).Return(request::StringResponse(
                200,
                json{
                    {"name", "somefolder"},
                    {"parentReference", {{"path", "/drive/root:/some/folder"}}},
                    {"folder", {{"childCount", 0}}}}
                    .dump(),
                "application/json"));

            WHEN("calling create_directory(somefolder)") {
                const auto newFolder = directory->create_directory("somefolder");
                THEN("the children endpoint should have been called with a json payload telling the new folder name") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(
                        0,
                        url == "https://graph.microsoft.com/v1.0/me/"
                               "drive/root:/some/folder:/children");
                    REQUIRE_REQUEST(0, body == "{\"@microsoft.graph.conflictBehavior\":\"fail\",\"folder\":{},\"name\":\"somefolder\"}");
                    REQUIRE_REQUEST(0, headers.at("Content-Type") == Request::MIMETYPE_JSON);
                }

                THEN("the new folder should be returned") {
                    REQUIRE(newFolder->name() == "somefolder");
                    REQUIRE(newFolder->path() == "/some/folder/somefolder");
                }
            }
        }
    }
}
