#include "webdav/WebdavFile.hpp"
#include "CloudSync/Cloud.hpp"
#include "CloudSync/Exceptions.hpp"
#include "request/Request.hpp"
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
using P = request::Request::ParameterType;

std::string xmlResponseContent(const std::string &eTag) {
    return "<?xml version=\"1.0\"?>"
           "<d:multistatus xmlns:d=\"DAV:\" xmlns:s=\"http://sabredav.org/ns\" xmlns:oc=\"http://owncloud.org/ns\" "
           "xmlns:nc=\"http://nextcloud.org/ns\">"
           "    <d:response>"
           "        <d:href>/file.txt</d:href>"
           "        <d:propstat>"
           "            <d:prop>"
           "                <d:getetag>&quot;" + eTag + "&quot;</d:getetag>"
           "            </d:prop>"
           "            <d:status>HTTP/1.1 200 OK</d:status>"
           "        </d:propstat>"
           "    </d:response>"
           "</d:multistatus>";
}

SCENARIO("WebdavFile", "[file][webdav]") {
    const std::string BASE_URL = "http://cloud";
    INIT_REQUEST();

    GIVEN("a webdav file instance of a textfile") {
        const auto file = std::make_shared<WebdavFile>(
            BASE_URL,
            "/test.txt",
            request,
            "test.txt",
            "\"7f3805660b049baadd3bef287d7d346b\"");
        AND_GIVEN("a request that returns 204 and a new eTag") {
            WHEN_REQUEST().RESPOND(request::Response(204, "", "text/plain", {{"etag", "\"newRevision\""}}));

            WHEN("writing to the file") {
                const std::string newData = "awesome new data";
                file->write(newData);
                THEN("a PUT request should be made on the path pointing to the file") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "PUT");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/test.txt");
                    REQUIRE_REQUEST(0, body == newData);
                    REQUIRE_REQUEST(
                        0,
                        parameters.at(P::HEADERS).at("If-Match") == "\"7f3805660b049baadd3bef287d7d346b\"");
                }
                THEN("the file should have a new Revision") {
                    REQUIRE(file->revision() == "\"newRevision\"");
                }
            }
        }
        AND_GIVEN("a request that returns 200") {
            WHEN_REQUEST().RESPOND(request::Response(200, "testtext", "text/plain"));

            WHEN("reading from a file") {
                std::string data = file->read();
                THEN("a GET request should be made on the desired file") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/test.txt");
                }
            }
        }
        AND_GIVEN("a request that returns 204") {
            WHEN_REQUEST().RESPOND(request::Response(204));
            WHEN("deleting the file") {
                file->rm();
                THEN("a DELETE request should be done on the resource") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, url == BASE_URL + "/test.txt");
                }
            }
        }

        AND_GIVEN("a PROPFIND request that returns the same etag (no change)") {
            WHEN_REQUEST().RESPOND(
                request::Response(200, xmlResponseContent("7f3805660b049baadd3bef287d7d346b"), "application/xml"));

            WHEN("calling pollChange()") {
                const bool hasChanged = file->pollChange();
                THEN("a PROPFIND request should be made to the resource asking for the etag") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "PROPFIND");
                    REQUIRE_REQUEST(0, url == BASE_URL + "/test.txt");
                    REQUIRE_REQUEST(
                        0,
                        body == "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
                                "<d:propfind xmlns:d=\"DAV:\">\n"
                                "   <d:prop>\n"
                                "       <d:getetag />\n"
                                "   </d:prop>\n"
                                "</d:propfind>");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Depth") == "0");
                }
                THEN("false should be returned") {
                    REQUIRE(hasChanged == false);
                }
            }
        }
        AND_GIVEN("a request that returns a new etag (change)") {
            WHEN_REQUEST().RESPOND(
                request::Response(200, xmlResponseContent("1ab803660mm49baads3bef287d7d3466"), "application/xml"));

            WHEN("calling pollChange()") {
                const bool hasChanged = file->pollChange();
                THEN("true should be returned") {
                    REQUIRE(hasChanged);
                }
                THEN("the file should have a new revision") {
                    REQUIRE(file->revision() == "\"1ab803660mm49baads3bef287d7d3466\"");
                }
            }
        }
        AND_GIVEN("a request that returns an invalid xml (not paresable)") {
            WHEN_REQUEST().RESPOND(request::Response(200, "thisisnotxml", "text/plain"));

            WHEN("calling pollChange()") {
                THEN("an InvalidResponse Exception should be thrown") {
                    REQUIRE_THROWS_AS(file->pollChange(), CloudSync::Cloud::InvalidResponse);
                }
            }
        }
        AND_GIVEN("a request that returns valid xml that misses the etag field") {
            WHEN_REQUEST().RESPOND(request::Response(200, "<?xml version=\"1.0\"?><a>a</a>", "text/plain"));

            WHEN("calling pollChange()") {
                THEN("an InvalidResponse Exception should be thrown") {
                    REQUIRE_THROWS_AS(file->pollChange(), CloudSync::Cloud::InvalidResponse);
                }
            }
        }
    }
}
