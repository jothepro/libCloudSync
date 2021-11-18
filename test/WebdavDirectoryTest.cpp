#include "webdav/WebdavDirectory.hpp"
#include "CloudSync/Cloud.hpp"
#include "CloudSync/Exceptions.hpp"
#include "request/Request.hpp"
#include "macros/access_protected.hpp"
#include "macros/request_mock.hpp"
#include "macros/shared_ptr_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <sstream>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;
using namespace CloudSync::webdav;
using namespace CloudSync::request;
using namespace CloudSync::request;
using P = request::Request::ParameterType;

SCENARIO("WebdavDirectory", "[directory][webdav]") {
    const std::string BASE_URL = "http://cloud";
    std::string xmlQuery = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
                           "<d:propfind  xmlns:d=\"DAV:\">\n"
                           "   <d:prop>\n"
                           "       <d:getlastmodified />\n"
                           "       <d:getetag />\n"
                           "       <d:getcontenttype />\n"
                           "       <d:resourcetype />\n"
                           "       <d:getcontentlength />\n"
                           "   </d:prop>\n"
                           "</d:propfind>";
    INIT_REQUEST();

    GIVEN("a webdav root directory") {

        const auto directory = std::make_shared<WebdavDirectory>(BASE_URL, "", "/", request, "");

        THEN("the a directory with path '/' & name '' should be returned") {
            REQUIRE(directory->path == "/");
            REQUIRE(directory->name == "");
        }

        AND_GIVEN("a PROPFIND request that returns a valid webdav directory description (Depth:0)") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                "<?xml version=\"1.0\"?>\n"
                "<d:multistatus xmlns:d=\"DAV:\" >\n"
                // directory description
                "  <d:response>\n"
                "      <d:href>/folder/</d:href>\n"
                // properties for the resource
                "      <d:propstat>\n"
                "          <d:prop>\n"
                "              <d:getlastmodified>Fri, 10 Jan 2020 20:42:38 GMT</d:getlastmodified>\n"
                "              <d:getetag>&quot;5e18e1bede073&quot;</d:getetag>\n"
                // this is a folder
                "              <d:resourcetype><d:collection/></d:resourcetype>\n"
                "          </d:prop>\n"
                "          <d:status>HTTP/1.1 200 OK</d:status>\n"
                "      </d:propstat>\n"
                // properties that have not been found for the resource
                "      <d:propstat>\n"
                "          <d:prop><d:getcontenttype/></d:prop>\n"
                "          <d:status>HTTP/1.1 404 Not Found</d:status>\n"
                "      </d:propstat>\n"
                "  </d:response>\n"
                "</d:multistatus>",
                "application/xml"));

            WHEN("calling cd(folder), cd(folder/), cd(/folder/), cd(/folder/test/..)") {
                std::string path = GENERATE(as<std::string>{}, "folder", "folder/", "/folder/", "/folder/test/..");
                const auto newDirectory = directory->cd(path);
                THEN("a PROPFIND request on the desired folder should be made") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "PROPFIND");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/folder");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Depth") == "0");
                    REQUIRE_REQUEST(0, body == xmlQuery);
                }
                THEN("a object representing the new directory should be returned") {
                    REQUIRE(newDirectory->name == "folder");
                    REQUIRE(newDirectory->path == "/folder");
                }
            }
        }
        AND_GIVEN("a request that returns 201 and a requset that returns a PROPFIND result") {
            WHEN_REQUEST()
                .RESPOND(request::Response(201))
                .RESPOND(request::Response(
                    200,
                    "<?xml version=\"1.0\"?>\n"
                    "<d:multistatus xmlns:d=\"DAV:\" >\n"
                    // directory description
                    "  <d:response>\n"
                    "      <d:href>/newDirectory/</d:href>\n"
                    // properties for the resource
                    "      <d:propstat>\n"
                    "          <d:prop>\n"
                    "              <d:getlastmodified>Fri, 10 Jan 2020 20:42:38 GMT</d:getlastmodified>\n"
                    "              <d:getetag>&quot;5e18e1bede073&quot;</d:getetag>\n"
                    "              <d:resourcetype><d:collection/></d:resourcetype>\n"
                    "          </d:prop>\n"
                    "          <d:status>HTTP/1.1 200 OK</d:status>\n"
                    "      </d:propstat>\n"
                    "      <d:propstat>\n"
                    "          <d:prop><d:getcontenttype/></d:prop>\n"
                    "          <d:status>HTTP/1.1 404 Not Found</d:status>\n"
                    "      </d:propstat>\n"
                    "  </d:response>\n"
                    "</d:multistatus>",
                    "application/xml"));

            WHEN("calling mkdir(newDirectory), mkdir(newDirectory/)") {
                std::string path = GENERATE(as<std::string>{}, "newDirectory", "newDirectory/");
                const auto newDirectory = directory->mkdir(path);
                THEN("a MKCOL request should be made on the path of the new folder") {
                    REQUIRE_REQUEST_CALLED().Twice();
                    REQUIRE_REQUEST(0, verb == "MKCOL");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/newDirectory");
                }
                THEN("a PROPFIND request on the new folder should be made") {
                    REQUIRE_REQUEST_CALLED().Twice();
                    REQUIRE_REQUEST(1, verb == "PROPFIND");
                    REQUIRE_REQUEST(1, url == BASE_URL + "/newDirectory");
                    REQUIRE_REQUEST(1, parameters.at(P::HEADERS).at("Depth") == "0");
                    REQUIRE_REQUEST(1, body == xmlQuery);
                }
                THEN("a object representing the new directory should be "
                     "returned") {
                    REQUIRE(newDirectory->name == "newDirectory");
                    REQUIRE(newDirectory->path == "/newDirectory");
                }
            }
        }
        AND_GIVEN("a DELETE request that returns 204") {
            WHEN_REQUEST().RESPOND(request::Response(204));

            WHEN("deleting the directory") {
                directory->rmdir();
                THEN("a DELETE request should be made on the current folder") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "DELETE");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/");
                }
            }
        }
        AND_GIVEN("a request that returns 201 and then a request that returns a description of the a new file") {
            WHEN_REQUEST()
                .RESPOND(request::Response(201))
                .RESPOND(request::Response(
                    200,
                    "<?xml version=\"1.0\"?>\n"
                    "<d:multistatus xmlns:d=\"DAV:\" >\n"
                    // file
                    "  <d:response>\n"
                    "      <d:href>/newfile.txt</d:href>\n"
                    // properties for the resource
                    "      <d:propstat>\n"
                    "          <d:prop>\n"
                    "              <d:getlastmodified>Fri, 10 Jan 2020 20:42:38 GMT</d:getlastmodified>\n"
                    "              <d:getetag>&quot;5e18e1bede073&quot;</d:getetag>\n"
                    // this is a folder
                    "              <d:getcontenttype>text/plain</d:getcontenttype>\n"
                    "              <d:resourcetype/>\n"
                    "          </d:prop>\n"
                    "          <d:status>HTTP/1.1 200 OK</d:status>\n"
                    "      </d:propstat>\n"
                    "  </d:response>\n"
                    "</d:multistatus>",
                    "application/xml"));

            WHEN("calling touch(newfile.txt)") {
                const auto newFile = directory->touch("newfile.txt");
                THEN("a PUT request should be made on path of the file to be created") {
                    REQUIRE_REQUEST_CALLED().Twice();
                    REQUIRE_REQUEST(0, verb == "PUT");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/newfile.txt");
                    REQUIRE_REQUEST(0, body == "");
                }
                THEN("a PROPFIND request should be made on the new file") {
                    REQUIRE_REQUEST_CALLED().Twice();
                    REQUIRE_REQUEST(1, verb == "PROPFIND");
                    REQUIRE_REQUEST(1, url == BASE_URL + "/newfile.txt");
                    REQUIRE_REQUEST(1, parameters.at(P::HEADERS).at("Depth") == "0");
                    REQUIRE_REQUEST(1, body == xmlQuery);
                }
                THEN("an object representing the file should be returned") {
                    REQUIRE(newFile->name == "newfile.txt");
                    REQUIRE(newFile->path == "/newfile.txt");
                    REQUIRE(newFile->revision() == "\"5e18e1bede073\"");
                }
            }
        }

        AND_GIVEN("a request that returns a valid root webdav directory listing") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                "<?xml version=\"1.0\"?>"
                "<d:multistatus xmlns:d=\"DAV:\" >"
                // root directory
                "  <d:response>"
                "      <d:href>/</d:href>"
                // properties for the resource
                "      <d:propstat>"
                "          <d:prop>"
                "              <d:getlastmodified>Fri, 10 Jan 2020 20:42:38 GMT</d:getlastmodified>"
                "              <d:getetag>&quot;5e18e1bede073&quot;</d:getetag>"
                // this is a folder
                "              <d:resourcetype><d:collection/></d:resourcetype>"
                "          </d:prop>"
                "          <d:status>HTTP/1.1 200 OK</d:status>"
                "      </d:propstat>"
                // properties that have not been found for the resource
                "      <d:propstat>"
                "          <d:prop><d:getcontenttype/></d:prop>"
                "          <d:status>HTTP/1.1 404 Not Found</d:status>"
                "      </d:propstat>"
                "  </d:response>"

                // subfolder
                "  <d:response>"
                "      <d:href>/subfolder/</d:href>"
                // properties for the resource
                "      <d:propstat>"
                "          <d:prop>"
                "              <d:getlastmodified>Fri, 10 Jan 2020 20:42:38 GMT</d:getlastmodified>"
                "              <d:getetag>&quot;5e18e1bede073&quot;</d:getetag>"
                // this is a folder
                "              <d:resourcetype><d:collection/></d:resourcetype>"
                "          </d:prop>"
                "          <d:status>HTTP/1.1 200 OK</d:status>"
                "      </d:propstat>"
                // properties that have not been found for the resource
                "      <d:propstat>"
                "          <d:prop><d:getcontenttype/></d:prop>"
                "          <d:status>HTTP/1.1 404 Not Found</d:status>"
                "      </d:propstat>"
                "  </d:response>"

                // file
                "  <d:response>"
                "      <d:href>/somefile.txt</d:href>"
                // properties for the resource
                "      <d:propstat>"
                "          <d:prop>"
                "              <d:getlastmodified>Fri, 10 Jan 2020 20:42:38 GMT</d:getlastmodified>"
                "              <d:getetag>&quot;5e18e1bede073&quot;</d:getetag>"
                // this is a folder
                "              <d:getcontenttype>text/plain</d:getcontenttype>"
                "              <d:resourcetype/>"
                "          </d:prop>"
                "          <d:status>HTTP/1.1 200 OK</d:status>"
                "      </d:propstat>"
                "  </d:response>"
                "</d:multistatus>",
                "application/xml"));

            WHEN("calling ls()") {
                const auto dirlist = directory->ls();
                THEN("a PROPFIND request should be made on the current directory") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "PROPFIND");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Depth") == "1");
                    REQUIRE_REQUEST(0, body == xmlQuery);
                }
                THEN("a list of 2 resources should be returned") {
                    REQUIRE(dirlist.size() == 2);
                }
                THEN("the first item should be a directory named 'subfolder/'") {
                    REQUIRE(dirlist[0]->name == "subfolder");
                    REQUIRE(dirlist[0]->path == "/subfolder");
                }
                THEN("the second item should be a textfile named 'somefile.txt'") {
                    const auto file = std::dynamic_pointer_cast<File>(dirlist[1]);
                    REQUIRE(file->name == "somefile.txt");
                    REQUIRE(file->path == "/somefile.txt");
                    REQUIRE(file->revision() == "\"5e18e1bede073\"");
                }
            }
        }
        AND_GIVEN("a PROPFIND request that returns a valid file description") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                "<?xml version=\"1.0\"?>"
                "<d:multistatus xmlns:d=\"DAV:\" >"
                // file
                "  <d:response>"
                "      <d:href>/some/path/somefile.txt</d:href>"
                "      <d:propstat>"
                "          <d:prop>"
                "              <d:getlastmodified>Fri, 10 Jan 2020 20:42:38 GMT</d:getlastmodified>"
                "              <d:getetag>&quot;5e18e1bede073&quot;</d:getetag>"
                "              <d:getcontenttype>text/plain</d:getcontenttype>"
                "              <d:resourcetype/>"
                "          </d:prop>"
                "          <d:status>HTTP/1.1 200 OK</d:status>"
                "      </d:propstat>"
                "  </d:response>"
                "</d:multistatus>",
                "application/xml"));

            WHEN("calling file(/some/path/somefile.txt)") {
                const auto file = directory->file("some/path/somefile.txt");
                THEN("a PROPFIND request should be made to the requested file") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "PROPFIND");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/some/path/somefile.txt");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Depth") == "0");
                    REQUIRE_REQUEST(0, body == xmlQuery);
                }
                THEN("the file should be returned") {
                    REQUIRE(file->name == "somefile.txt");
                    REQUIRE(file->path == "/some/path/somefile.txt");
                    REQUIRE(file->revision() == "\"5e18e1bede073\"");
                }
            }
        }
        AND_GIVEN("a PROPFIND request that returns a xml that misses the expected content") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                "<!xml version=\"1.0\"?>"
                "<notwhatweexpect />",
                "application/xml"));

            WHEN("calling ls()") {
                THEN("a RequestException should be thrown") {
                    REQUIRE_THROWS_AS(directory->ls(), CloudSync::Cloud::InvalidResponse);
                }
            }
            WHEN("calling cd(test)") {
                THEN("a RequestException should be thrown") {
                    REQUIRE_THROWS_AS(directory->cd("test"), CloudSync::Cloud::InvalidResponse);
                }
            }
        }
        AND_GIVEN("a PROPFIND request that returns no xml") {
            WHEN_REQUEST().RESPOND(request::Response(200, "noxml", "text/plain"));

            WHEN("getting the current dir content") {
                THEN("a RequestException should be thrown") {
                    REQUIRE_THROWS_AS(directory->ls(), CloudSync::Cloud::InvalidResponse);
                }
            }
            WHEN("calling cd(test)") {
                THEN("a RequestException should be thrown") {
                    REQUIRE_THROWS_AS(directory->cd("test"), CloudSync::Cloud::InvalidResponse);
                }
            }
        }
    }

    GIVEN("a webdav directory (non-root)") {
        const auto directory = std::make_shared<WebdavDirectory>(BASE_URL, "", "/some/folder", request, "folder");

        THEN("the a directory with path '/some/folder' & name 'folder' should "
             "be returned") {
            REQUIRE(directory->path == "/some/folder");
            REQUIRE(directory->name == "folder");
        }

        AND_GIVEN("a PROPFIND request that returns a valid webdav directory description (Depth:0)") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                "<?xml version=\"1.0\"?>"
                "<d:multistatus xmlns:d=\"DAV:\" >"
                // directory description
                "  <d:response>"
                "      <d:href>/some/folder/somefolder/</d:href>"
                // properties for the resource
                "      <d:propstat>"
                "          <d:prop>"
                "              <d:getlastmodified>Fri, 10 Jan 2020 20:42:38 GMT</d:getlastmodified>"
                "              <d:getetag>&quot;5e18e1bede073&quot;</d:getetag>"
                "              <d:resourcetype><d:collection/></d:resourcetype>"
                "          </d:prop>"
                "          <d:status>HTTP/1.1 200 OK</d:status>"
                "      </d:propstat>"
                // properties that have not been found for the resource
                "      <d:propstat>"
                "          <d:prop><d:getcontenttype/></d:prop>"
                "          <d:status>HTTP/1.1 404 Not Found</d:status>"
                "      </d:propstat>"
                "  </d:response>"
                "</d:multistatus>",
                "application/xml"));

            WHEN("calling cd(somefolder), cd(somefolder/), cd(somefolder/morefolder/..), cd(/somefolder/)") {
                std::string path = GENERATE(
                    as<std::string>{},
                    "somefolder",
                    "somefolder/",
                    "somefolder/morefolder/..");
                const auto newDirectory = directory->cd(path);
                THEN("a PROPFIND request on the desired folder should be made") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "PROPFIND");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/some/folder/somefolder");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Depth") == "0");
                    REQUIRE_REQUEST(0, body == xmlQuery);
                }
                THEN("a object representing the new directory should be "
                     "returned") {
                    REQUIRE(newDirectory->name == "somefolder");
                    REQUIRE(newDirectory->path == "/some/folder/somefolder");
                }
            }
        }
    }

    GIVEN("a webdav directory with a nextcloud/owncloud dirOffset") {
        const auto nextcloudDir =
            std::make_shared<WebdavDirectory>(BASE_URL, "/remote.php/webdav", "/some/folder", request, "folder");

        AND_GIVEN("a PROPFIND request that returns a valid webdav directory description (Depth:0) including the "
                  "dirOffset of nextcloud") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                "<?xml version=\"1.0\"?>"
                "<d:multistatus xmlns:d=\"DAV:\" >"
                // directory description
                "  <d:response>"
                "      <d:href>/remote.php/webdav/some/folder/somefolder/</d:href>"
                // properties for the resource
                "      <d:propstat>"
                "          <d:prop>"
                "              <d:getlastmodified>Fri, 10 Jan 2020 20:42:38 GMT</d:getlastmodified>"
                "              <d:getetag>&quot;5e18e1bede073&quot;</d:getetag>"
                // this is a folder
                "              <d:resourcetype><d:collection/></d:resourcetype>"
                "          </d:prop>"
                "          <d:status>HTTP/1.1 200 OK</d:status>"
                "      </d:propstat>"
                // properties that have not been found for the resource
                "      <d:propstat>"
                "          <d:prop><d:getcontenttype/></d:prop>"
                "          <d:status>HTTP/1.1 404 Not Found</d:status>"
                "      </d:propstat>"
                "  </d:response>"
                "</d:multistatus>",
                "application/xml"));

            WHEN("calling cd(somefolder)") {
                const auto newNextcloudDir = nextcloudDir->cd("somefolder");
                THEN("a PROPFIND request should be made on the correct resource Path") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "PROPFIND");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/remote.php/webdav/some/folder/somefolder");
                    REQUIRE_REQUEST(0, body == xmlQuery);
                }

                THEN("the folder 'somefolder' should be returned with the path '/some/folder/somefolder'") {
                    REQUIRE(newNextcloudDir->name == "somefolder");
                    REQUIRE(newNextcloudDir->path == "/some/folder/somefolder");
                }
            }
        }
    }
}
