#include "webdav/WebdavFile.hpp"
#include "CloudSync/Cloud.hpp"
#include "CloudSync/exceptions/cloud/CloudException.hpp"
#include "request/Request.hpp"
#include "macros/request_mock.hpp"
#include "macros/basic_auth_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <sstream>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;
using namespace CloudSync::webdav;
using namespace CloudSync::request;

std::string xmlResponseContent(const std::string &eTag) {
    return "<?xml version=\"1.0\"?>"
           "<d:multistatus xmlns:d=\"DAV:\" xmlns:s=\"http://sabredav.org/ns\" xmlns:oc=\"http://owncloud.org/ns\" "
           "xmlns:nc=\"http://nextcloud.org/ns\">"
               "<d:response>"
                   "<d:href>/file.txt</d:href>"
                   "<d:propstat>"
                       "<d:prop>"
                           "<d:getetag>&quot;" + eTag + "&quot;</d:getetag>"
                       "</d:prop>"
                       "<d:status>HTTP/1.1 200 OK</d:status>"
                   "</d:propstat>"
               "</d:response>"
           "</d:multistatus>";
}

SCENARIO("WebdavFile", "[file][webdav]") {
    const std::string BASE_URL = "http://cloud";
    INIT_REQUEST();
    BASIC_AUTH_MOCK("john", "password123");
    GIVEN("a webdav file instance of a textfile") {
        const auto file = std::make_shared<WebdavFile>(
            BASE_URL,
            "/test.txt",
            credentials,
            request,
            "test.txt",
            "\"7f3805660b049baadd3bef287d7d346b\"");
        AND_GIVEN("a request that returns 204 and a new eTag") {
            When(Method(requestMock, request)).Return(request::StringResponse(204, "", "text/plain", {{"etag", "\"newRevision\""}}));

            WHEN("writing to the file") {
                const std::string newData = "awesome new data";
                file->write(newData);
                THEN("a PUT request should be made on the path pointing to the file") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "PUT");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/test.txt");
                    REQUIRE_REQUEST(0, body == newData);
                    REQUIRE_REQUEST(
                        0,
                        headers.at("If-Match") == "\"7f3805660b049baadd3bef287d7d346b\"");
                }
                THEN("the file should have a new Revision") {
                    REQUIRE(file->revision() == "\"newRevision\"");
                }
            }
        }
        AND_GIVEN("a request that returns 200 and some text in the body") {
            When(Method(requestMock, request)).Return(request::StringResponse(200, "testtext", "text/plain"));

            WHEN("reading from a file") {
                const auto data = file->read();
                THEN("a GET request should be made on the desired file") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/test.txt");
                }
                THEN("The read data should be 'testtext'") {
                    REQUIRE(data == "testtext");
                }
            }
        }
        AND_GIVEN("a request that returns 204") {
            When(Method(requestMock, request)).Return(request::StringResponse(204));
            WHEN("deleting the file") {
                file->remove();
                THEN("a DELETE request should be made on the resource") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, url == BASE_URL + "/test.txt");
                }
            }
        }

        AND_GIVEN("a request that returns 200 and the same etag (no change)") {
            When(Method(requestMock, request)).Return(
                request::StringResponse(200, xmlResponseContent("7f3805660b049baadd3bef287d7d346b"), "application/xml"));

            WHEN("calling poll_change()") {
                const bool hasChanged = file->poll_change();
                THEN("a PROPFIND request should be made to the resource asking for the etag") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "PROPFIND");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/test.txt");
                    REQUIRE_REQUEST(
                        0,
                        body == "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                "<d:propfind xmlns:d=\"DAV:\">"
                                    "<d:prop>"
                                        "<d:getetag/>"
                                    "</d:prop>"
                                "</d:propfind>");
                    REQUIRE_REQUEST(0, headers.at("Depth") == "0");
                }
                THEN("false should be returned") {
                    REQUIRE(hasChanged == false);
                }
            }
        }
        AND_GIVEN("a request that returns 200 and a new etag (change)") {
            When(Method(requestMock, request)).Return(
                request::StringResponse(200, xmlResponseContent("1ab803660mm49baads3bef287d7d3466"), "application/xml"));

            WHEN("calling poll_change()") {
                const bool hasChanged = file->poll_change();
                THEN("true should be returned") {
                    REQUIRE(hasChanged);
                }
                THEN("the file should have a new revision") {
                    REQUIRE(file->revision() == "\"1ab803660mm49baads3bef287d7d3466\"");
                }
            }
        }
        AND_GIVEN("a request that returns an invalid xml (not paresable)") {
            When(Method(requestMock, request)).Return(request::StringResponse(200, "thisisnotxml", "text/plain"));

            WHEN("calling poll_change()") {
                THEN("an InvalidResponse Exception should be thrown") {
                    REQUIRE_THROWS_AS(file->poll_change(), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
        }
        AND_GIVEN("a request that returns valid xml that misses the etag field") {
            When(Method(requestMock, request)).Return(request::StringResponse(200, "<?xml version=\"1.0\"?><a>a</a>", "text/plain"));

            WHEN("calling poll_change()") {
                THEN("an InvalidResponse Exception should be thrown") {
                    REQUIRE_THROWS_AS(file->poll_change(), CloudSync::exceptions::cloud::InvalidResponse);
                }
            }
        }
    }
}
