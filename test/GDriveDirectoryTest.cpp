#include "gdrive/GDriveDirectory.hpp"
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
using P = Request::ParameterType;

SCENARIO("GDriveDirectory", "[directory][gdrive]") {
    INIT_REQUEST()
    const std::string BASE_URL = "https://www.googleapis.com/drive/v2";

    GIVEN("a google drive root directory") {
        const auto directory = std::make_shared<GDriveDirectory>(BASE_URL, "root", "root", "root", "/", request, "");

        THEN("the working dir should be '/'") {
            REQUIRE(directory->pwd() == "/");
        }

        AND_GIVEN("a request that returns a valid folder listing") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{{"items",
                      {
                          {{"kind", "drive#file"},
                           {"id", "1dInfWIELU8Hc1sP_bsGnVa44DgBpNybI"},
                           {"title", "testfolder"},
                           {"mimeType", "application/vnd.google-apps.folder"},
                           {"etag", "2"},
                           {"parents", {{{"id", "0ANREbljg-Vs3Uk9PVA"}, {"isRoot", true}}}}},
                          {{"kind", "drive#file"},
                           {"id", "2iInfWIELU3tc1sPhbsGwVa34DgBpN3Es"},
                           {"title", "test.txt"},
                           {"mimeType", "text/plain"},
                           {"etag", "1"},
                           {"parents", {{{"id", "0ANREbljg-Vs3Uk9PVA"}, {"isRoot", true}}}}},
                      }}}
                    .dump(),
                "application/json"));

            WHEN("calling ls()") {
                const auto resourceList = directory->ls();
                THEN("the google drive files endpoint should be called with the correct query") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/files");
                    REQUIRE_REQUEST(
                        0,
                        parameters.at(P::QUERY_PARAMS).at("q") == "'root' in parents and trashed = false");
                    REQUIRE_REQUEST(
                        0,
                        parameters.at(P::QUERY_PARAMS).at("fields") ==
                            "items(kind,id,title,mimeType,etag,parents(id,isRoot))");
                }
                THEN("a list of 2 resources should be returned") {
                    REQUIRE(resourceList.size() == 2);
                    REQUIRE(resourceList[0]->name() == "testfolder");
                    REQUIRE(resourceList[0]->path() == "/testfolder");
                    const auto file = std::dynamic_pointer_cast<File>(resourceList[1]);
                    REQUIRE(file->name() == "test.txt");
                    REQUIRE(file->path() == "/test.txt");
                    REQUIRE(file->revision() == "1");
                }
            }
        }
        AND_GIVEN("a GET request that throws 401 unauthorized") {
            WHEN_REQUEST().Throw(request::Response::Unauthorized(""));

            WHEN("calling ls()") {
                THEN("an Cloud::AuthorizationFailed exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->ls(), Cloud::AuthorizationFailed);
                }
            }
        }
        AND_GIVEN("a GET request that a folder description") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{{"items",
                      {{{"kind", "drive#file"},
                        {"id", "1dInfWIELU8Hc1sP_bsGnVa44DgBpNybI"},
                        {"title", "testfolder"},
                        {"mimeType", "application/vnd.google-apps.folder"},
                        {"etag", "2"},
                        {"parents", {{{"id", "0ANREbljg-Vs3Uk9PVA"}, {"isRoot", true}}}}}}}}
                    .dump(),
                "application/json"));

            WHEN("calling cd(testfolder), cd(/testfolder/), cd(/subfolder/../testfolder)") {
                const std::string path =
                    GENERATE(as<std::string>{}, "testfolder", "/testfolder/", "/subfolder/../testfolder");
                const auto newDir = directory->cd(path);
                THEN("the google drive files endpoint should be called with the correct query") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/files");
                    REQUIRE_REQUEST(
                        0,
                        parameters.at(P::QUERY_PARAMS).at("q") ==
                            "'root' in parents and title = 'testfolder' and trashed = false");
                    REQUIRE_REQUEST(
                        0,
                        parameters.at(P::QUERY_PARAMS).at("fields") ==
                            "items(kind,id,title,mimeType,etag,parents(id,isRoot))");
                }
                THEN("the desired folder should be returned") {
                    REQUIRE(newDir->name() == "testfolder");
                    REQUIRE(newDir->path() == "/testfolder");
                }
            }
        }
        AND_GIVEN("a request that returns a file description") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{{"items",
                      {{{"kind", "drive#file"},
                        {"id", "52InfWIELUrHc1sPlbstnVa44DgBpNyb5"},
                        {"title", "test.txt"},
                        {"mimeType", "text/plain"},
                        {"etag", "2"},
                        {"parents", {{{"id", "0ANREbljg-Vs3Uk9PVA"}, {"isRoot", true}}}}}}}}
                    .dump(),
                "application/json"));

            WHEN("calling file(test.txt)") {
                const auto file = directory->file("test.txt");
                THEN("the google drive files endpoint should be called with the correct query parameters") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/files");
                    REQUIRE_REQUEST(
                        0,
                        parameters.at(P::QUERY_PARAMS).at("q") ==
                            "'root' in parents and title = 'test.txt' and trashed = false");
                    REQUIRE_REQUEST(
                        0,
                        parameters.at(P::QUERY_PARAMS).at("fields") ==
                            "items(kind,id,title,mimeType,etag,parents(id,isRoot))");
                }
                THEN("the desired file should be returned") {
                    REQUIRE(file->name() == "test.txt");
                    REQUIRE(file->path() == "/test.txt");
                    REQUIRE(file->revision() == "2");
                }
            }
        }
        AND_GIVEN("a POST request that returns a new folder resource description") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{
                    {"kind", "drive#file"},
                    {"id", "folderId"},
                    {"title", "newfolder"},
                    {"mimeType", "application/vnd.google-apps.folder"},
                    {"etag", "1"},
                    {"parents", {{{"id", "0ANREbljg-Vs3Uk9PVA"}, {"isRoot", true}}}}}
                    .dump(),
                "application/json"));

            WHEN("calling mkdir(newfolder)") {
                const auto newDir = directory->mkdir("newfolder");
                THEN("the endpoint for creating a new resource should be called") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/files");
                    REQUIRE_REQUEST(
                        0,
                        parameters.at(P::QUERY_PARAMS).at("fields") ==
                            "kind,id,title,mimeType,etag,parents(id,isRoot)");
                    REQUIRE_REQUEST(
                        0,
                        body == "{\"mimeType\":\"application/vnd.google-apps.folder\","
                                "\"parents\":[{\"id\":\"root\"}],\"title\":\"newfolder\"}");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Content-Type") == Request::MIMETYPE_JSON);
                }
                THEN("the new folder should be returned") {
                    REQUIRE(newDir->name() == "newfolder");
                    REQUIRE(newDir->path() == "/newfolder");
                }
            }
        }
        AND_GIVEN("a request that returns a new file resource description") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{
                    {"kind", "drive#file"},
                    {"id", "folderId"},
                    {"title", "newfile.txt"},
                    {"mimeType", "text/plain"},
                    {"etag", "1"},
                    {"parents", {{{"id", "0ANREbljg-Vs3Uk9PVA"}, {"isRoot", true}}}}}
                    .dump(),
                "application/json"));

            WHEN("calling touch(newfile.txt)") {
                const auto newFile = directory->touch("newfile.txt");
                THEN("the endpoint for creating a new resource should be called") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/files");
                    REQUIRE_REQUEST(
                        0,
                        parameters.at(P::QUERY_PARAMS).at("fields") ==
                            "kind,id,title,mimeType,etag,parents(id,isRoot)");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Content-Type") == Request::MIMETYPE_JSON);
                    REQUIRE_REQUEST(
                        0,
                        body == "{\"mimeType\":\"text/plain\","
                                "\"parents\":[{\"id\":\"root\"}],\"title\":\"newfile.txt\"}");
                }
                THEN("the new file resource should be returned") {
                    REQUIRE(newFile->name() == "newfile.txt");
                    REQUIRE(newFile->path() == "/newfile.txt");
                }
            }
        }
        WHEN("calling rmdir()") {
            THEN("a PermissionDenied should be called (deleting root is not allowed") {
                REQUIRE_THROWS_AS(directory->rmdir(), CloudSync::Resource::PermissionDenied);
            }
        }
    }
    GIVEN("a google drive directory (non root)") {
        const auto directory = std::make_shared<GDriveDirectory>(
            BASE_URL,
            "root",
            "resourceId",
            "parentId",
            "/test/folder",
            request,
            "folder");

        AND_GIVEN("a request that returns a valid folder description") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                json{
                    {"kind", "drive#file"},
                    {"id", "1dInfWIELU8Hc1sP_bsGnVa44DgBpNybI"},
                    {"title", "test"},
                    {"mimeType", "application/vnd.google-apps.folder"},
                    {"etag", "2"},
                    {"parents", {{{"id", "0ANREbljg-Vs3Uk9PVA"}, {"isRoot", false}}}}}
                    .dump(),
                "application/json"));

            WHEN("calling cd(..)") {
                const auto parentDir = directory->cd("..");
                THEN("a google drive lists request should be done with a query for the parent folder id") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/files/parentId");
                    REQUIRE_REQUEST(
                        0,
                        parameters.at(P::QUERY_PARAMS).at("fields") ==
                            "kind,id,title,mimeType,etag,parents(id,isRoot)");
                }

                THEN("the parent directory should be returned") {
                    REQUIRE(parentDir->name() == "test");
                    REQUIRE(parentDir->path() == "/test");
                }
            }
            WHEN("calling cd(../../") {
                const auto rootDir = directory->cd("../../");
                THEN("only one request should be made. The root cannot be queried") {
                    REQUIRE_REQUEST_CALLED().Once();
                }
                THEN("the root dir should be returned") {
                    REQUIRE(rootDir->name() == "");
                    REQUIRE(rootDir->path() == "/");
                }
            }
        }
        AND_GIVEN("a request that returns 204") {
            WHEN_REQUEST().RESPOND(request::Response(204));

            WHEN("calling rmdir()") {
                directory->rmdir();
                THEN("a google drive delete call should be made") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "DELETE");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/files/resourceId");
                }
            }
        }
        AND_GIVEN("a request that throws 404 Not Found") {
            WHEN_REQUEST().Throw(request::Response::NotFound(
                json{{"error", {{"errors", {{{"domain", "global"}, {"reason", "notFound"}}}}}}}.dump()));
            WHEN("calling rmdir()") {
                THEN("a NoSuchFileOrDirectory Exception should be thrown") {
                    REQUIRE_THROWS_AS(directory->rmdir(), Resource::NoSuchFileOrDirectory);
                }
            }
        }
    }
}
