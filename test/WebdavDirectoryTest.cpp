#include "webdav/WebdavDirectory.hpp"
#include "CloudSync/Cloud.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"
#include "CloudSync/exceptions/resource/ResourceException.hpp"
#include "request/Request.hpp"
#include "macros/request_mock.hpp"
#include "macros/basic_auth_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;
using namespace CloudSync::webdav;
using namespace CloudSync::request;
using namespace CloudSync::request;

SCENARIO("WebdavDirectory", "[directory][webdav]") {
    const std::string BASE_URL = "http://cloud";
    const std::string xmlQuery = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                           "<d:propfind  xmlns:d=\"DAV:\">"
                               "<d:prop>"
                                   "<d:getlastmodified/>"
                                   "<d:getetag/>"
                                   "<d:getcontenttype/>"
                                   "<d:resourcetype/>"
                                   "<d:getcontentlength/>"
                               "</d:prop>"
                           "</d:propfind>";
    INIT_REQUEST();
    BASIC_AUTH_MOCK("john", "password123");
    GIVEN("a webdav root directory") {

        const auto directory = std::make_shared<WebdavDirectory>(BASE_URL, "", "/", credentials, request, "");

        THEN("the a directory with path '/' & name '' should be returned") {
            REQUIRE(directory->path() == "/");
            REQUIRE(directory->name() == "");
        }

        AND_GIVEN("a PROPFIND request that returns a valid webdav directory description (Depth:0)") {
            When(Method(requestMock, request)).Return(request::StringResponse(
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

            WHEN("calling get_directory(folder), get_directory(folder/), get_directory(/folder/), get_directory(/folder/test/..)") {
                std::string path = GENERATE(as<std::string>{}, "folder", "folder/", "/folder/", "/folder/test/..");
                const auto new_directory = directory->get_directory(path);
                THEN("a PROPFIND request on the desired folder should be made") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "PROPFIND");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/folder");
                    REQUIRE_REQUEST(0, headers.at("Depth") == "0");
                    REQUIRE_REQUEST(0, body == xmlQuery);
                }
                THEN("a object representing the new directory should be returned") {
                    REQUIRE(new_directory->name() == "folder");
                    REQUIRE(new_directory->path() == "/folder");
                }
            }
        }
        AND_GIVEN("a request that returns 201") {
            When(Method(requestMock, request)).Return(request::StringResponse(201));

            WHEN("calling create_directory(newDirectory), create_directory(newDirectory/)") {
                std::string path = GENERATE(as<std::string>{}, "newDirectory", "newDirectory/");
                const auto new_directory = directory->create_directory(path);
                THEN("a MKCOL request should be made on the path of the new folder") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "MKCOL");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/newDirectory");
                }
                THEN("a object representing the new directory should be "
                     "returned") {
                    REQUIRE(new_directory->name() == "newDirectory");
                    REQUIRE(new_directory->path() == "/newDirectory");
                }
            }
        }
        WHEN("deleting the directory") {
            THEN("a PermissionDenied exeception should be thrown, because the root dir cannot be deleted") {
                REQUIRE_THROWS_AS(directory->remove(), CloudSync::exceptions::resource::PermissionDenied);
            }
        }
        AND_GIVEN("a request series  that returns 404 and then 201 with an etag header") {
            When(Method(requestMock, request)).Throw(request::exceptions::response::NotFound())
                .Return(request::StringResponse(201, "", "", {{"etag", "\"5e18e1bede073\""}}));

            WHEN("calling create_file(newfile.txt)") {
                const auto new_file = directory->create_file("newfile.txt");
                THEN("a HEAD request should be made to find out if the resource already exists") {
                    Verify(Method(requestMock, request)).Exactly(2);
                    REQUIRE_REQUEST(0, verb == "HEAD");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/newfile.txt");
                }
                THEN("a PUT request should be made on path of the file to be created") {
                    Verify(Method(requestMock, request)).Exactly(2);
                    REQUIRE_REQUEST(1, verb == "PUT");
                    REQUIRE_REQUEST(1, url == BASE_URL + "/newfile.txt");
                    REQUIRE_REQUEST(1, body == "");
                }
                THEN("an object representing the file should be returned") {
                    REQUIRE(new_file->name() == "newfile.txt");
                    REQUIRE(new_file->path() == "/newfile.txt");
                    REQUIRE(new_file->revision() == "\"5e18e1bede073\"");
                }
            }
        }

        AND_GIVEN("a request that returns a valid root webdav directory listing") {
            When(Method(requestMock, request)).Return(request::StringResponse(
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

            WHEN("calling list_resources()") {
                const auto dirlist = directory->list_resources();
                THEN("a PROPFIND request should be made on the current directory") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "PROPFIND");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/");
                    REQUIRE_REQUEST(0, headers.at("Depth") == "1");
                    REQUIRE_REQUEST(0, body == xmlQuery);
                }
                THEN("a list of 2 resources should be returned") {
                    REQUIRE(dirlist.size() == 2);
                }
                THEN("the first item should be a directory named 'subfolder/'") {
                    REQUIRE(dirlist[0]->name() == "subfolder");
                    REQUIRE(dirlist[0]->path() == "/subfolder");
                }
                THEN("the second item should be a textfile named 'somefile.txt'") {
                    const auto file = std::dynamic_pointer_cast<File>(dirlist[1]);
                    REQUIRE(file->name() == "somefile.txt");
                    REQUIRE(file->path() == "/somefile.txt");
                    REQUIRE(file->revision() == "\"5e18e1bede073\"");
                }
            }
        }
        AND_GIVEN("a request that returns a valid file description") {
            When(Method(requestMock, request)).Return(request::StringResponse(
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

            WHEN("calling get_file(some/path/somefile.txt)") {
                const auto file = directory->get_file("some/path/somefile.txt");
                THEN("a PROPFIND request should be made to the requested file") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "PROPFIND");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/some/path/somefile.txt");
                    REQUIRE_REQUEST(0, headers.at("Depth") == "0");
                    REQUIRE_REQUEST(0, body == xmlQuery);
                }
                THEN("the file should be returned") {
                    REQUIRE(file->name() == "somefile.txt");
                    REQUIRE(file->path() == "/some/path/somefile.txt");
                    REQUIRE(file->revision() == "\"5e18e1bede073\"");
                }
            }
            WHEN("calling get_directory(some/path/somefile.txt") {
                THEN("a NoSuchFileOrDirectory exception should be thrown") {
                    REQUIRE_THROWS_AS(
                            directory->get_directory("some/path/somefile.txt"),
                        CloudSync::exceptions::resource::NoSuchResource);
                }
            }
        }
        AND_GIVEN("a request that returns a xml that misses the expected content") {
            When(Method(requestMock, request)).Return(request::StringResponse(
                200,
                "<!xml version=\"1.0\"?>"
                "<notwhatweexpect />",
                "application/xml"));

            WHEN("calling list_resources()") {
                THEN("a RequestException should be thrown") {
                    REQUIRE_THROWS_AS(directory->list_resources(), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
            WHEN("calling get_directory(test)") {
                THEN("a RequestException should be thrown") {
                    REQUIRE_THROWS_AS(directory->get_directory("test"), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
        }
        AND_GIVEN("a request that returns no xml") {
            When(Method(requestMock, request)).Return(request::StringResponse(200, "noxml", "text/plain"));

            WHEN("getting the current dir content") {
                THEN("a RequestException should be thrown") {
                    REQUIRE_THROWS_AS(directory->list_resources(), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
            WHEN("calling get_directory(test)") {
                THEN("a RequestException should be thrown") {
                    REQUIRE_THROWS_AS(directory->get_directory("test"), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
        }
    }

    GIVEN("a webdav directory (non-root)") {
        const auto directory = std::make_shared<WebdavDirectory>(BASE_URL, "", "/some/folder", credentials, request, "folder");

        THEN("the a directory with path '/some/folder' & name 'folder' should "
             "be returned") {
            REQUIRE(directory->path() == "/some/folder");
            REQUIRE(directory->name() == "folder");
        }

        AND_GIVEN("a request that returns a valid webdav directory description (Depth:0)") {
            When(Method(requestMock, request)).Return(request::StringResponse(
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

            WHEN("calling get_directory(somefolder), get_directory(somefolder/), get_directory(somefolder/morefolder/..), get_directory(/somefolder/)") {
                std::string path = GENERATE(
                    as<std::string>{},
                    "somefolder",
                    "somefolder/",
                    "somefolder/morefolder/..");
                const auto newDirectory = directory->get_directory(path);
                THEN("a PROPFIND request on the desired folder should be made") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "PROPFIND");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/some/folder/somefolder");
                    REQUIRE_REQUEST(0, headers.at("Depth") == "0");
                    REQUIRE_REQUEST(0, body == xmlQuery);
                }
                THEN("a object representing the new directory should be "
                     "returned") {
                    REQUIRE(newDirectory->name() == "somefolder");
                    REQUIRE(newDirectory->path() == "/some/folder/somefolder");
                }
            }
        }
    }

    GIVEN("a webdav directory with a nextcloud/owncloud dirOffset") {
        const auto nextcloudDir =
            std::make_shared<WebdavDirectory>(BASE_URL, "/remote.php/webdav", "/some/folder", credentials, request, "folder");
        AND_GIVEN("a request that returns 204") {
            When(Method(requestMock, request)).Return(request::StringResponse(204, ""));

            WHEN("deleting the directory") {
                nextcloudDir->remove();
                THEN("a DELETE request should be made on the current folder") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "DELETE");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/remote.php/webdav/some/folder");
                }
            }
        }
        AND_GIVEN("a request that returns a valid webdav directory description (Depth:0) including the "
                  "dirOffset of nextcloud") {
            When(Method(requestMock, request)).Return(request::StringResponse(
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

            WHEN("calling get_directory(somefolder)") {
                const auto newNextcloudDir = nextcloudDir->get_directory("somefolder");
                THEN("a PROPFIND request should be made on the correct resource Path") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "PROPFIND");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/remote.php/webdav/some/folder/somefolder");
                    REQUIRE_REQUEST(0, body == xmlQuery);
                }

                THEN("the folder 'somefolder' should be returned with the path '/some/folder/somefolder'") {
                    REQUIRE(newNextcloudDir->name() == "somefolder");
                    REQUIRE(newNextcloudDir->path() == "/some/folder/somefolder");
                }
            }
        }
    }
}
