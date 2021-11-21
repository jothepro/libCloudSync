#include "nextcloud/NextcloudCloud.hpp"
#include "request/Request.hpp"
#include "macros/request_mock.hpp"
#include "macros/shared_ptr_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;
using namespace CloudSync::request;
using P = request::Request::ParameterType;

SCENARIO("NextcloudCloud", "[cloud][nextcloud]") {
    INIT_REQUEST();
    GIVEN("a nextcloud cloud instance") {
        const auto cloud = std::make_shared<nextcloud::NextcloudCloud>("http://nextcloud", request);
        AND_GIVEN("a request that returns a ocs user description") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                "<?xml version=\"1.0\"?>"
                "<ocs>"
                "  <data>"
                "    <display-name>John Doe</display-name>"
                "  </data>"
                "</ocs>",
                "application/xml"));

            WHEN("calling getUserDisplayName()") {
                std::string name = cloud->getUserDisplayName();
                THEN("the display name should be set to 'John Doe'") {
                    REQUIRE(name == "John Doe");
                }
                THEN("a request to the OCS endpoint should be made") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "http://nextcloud/ocs/v1.php/cloud/user");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("OCS-APIRequest") == "true");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("Accept") == Request::MIMETYPE_XML);
                }
            }
        }
        WHEN("calling getBaseUrl") {
            const auto baseUrl = cloud->getBaseUrl();
            THEN("it should return the correct base url") {
                REQUIRE(baseUrl == "http://nextcloud");
            }
        }
        WHEN("calling getTokenUrl") {
            const std::string result = cloud->getTokenUrl();
            THEN("an empty string should be returned") {
                REQUIRE(result.empty());
            }
        }
        WHEN("calling root()") {
            const auto rootDir = cloud->root();
            THEN("a root dir with path '/' & name '' should be returned") {
                REQUIRE(rootDir->name() == "");
                REQUIRE(rootDir->path() == "/");
            }
        }
        AND_GIVEN("a request that returns an OCS success response") {
            WHEN_REQUEST().RESPOND(request::Response(
                200,
                "<?xml version=\"1.0\"?>\n"
                "<ocs>\n"
                "    <meta>\n"
                "        <status>ok</status>\n"
                "        <statuscode>200</statuscode>\n"
                "        <message>OK</message>\n"
                "    </meta>\n"
                "    <data/>\n"
                "</ocs>",
                "application/xml"));
            WHEN("calling logout()") {
                cloud->logout();
                THEN("the app-password should be invalidated") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "DELETE");
                    REQUIRE_REQUEST(0, url == "http://nextcloud/ocs/v2.php/core/apppassword");
                    REQUIRE_REQUEST(0, parameters.at(P::HEADERS).at("OCS-APIRequest") == "true");
                }
            }
        }
    }
}
