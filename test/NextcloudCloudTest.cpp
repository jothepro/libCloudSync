#include "nextcloud/NextcloudCloud.hpp"
#include "request/Request.hpp"
#include "macros/request_mock.hpp"
#include "macros/basic_auth_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;
using namespace CloudSync::request;

SCENARIO("NextcloudCloud", "[cloud][nextcloud]") {
    INIT_REQUEST();
    BASIC_AUTH_MOCK("john", "password123");
    GIVEN("a nextcloud cloud instance") {
        const auto cloud = std::make_shared<nextcloud::NextcloudCloud>("http://nextcloud", credentials, request);
        AND_GIVEN("a request that returns a ocs user description") {
            When(Method(requestMock, request)).Return(request::StringResponse(
                200,
                "<?xml version=\"1.0\"?>"
                "<ocs>"
                "  <data>"
                "    <display-name>John Doe</display-name>"
                "  </data>"
                "</ocs>",
                "application/xml"));

            WHEN("calling get_user_display_name()") {
                const std::string name = cloud->get_user_display_name();
                THEN("the display name should be set to 'John Doe'") {
                    REQUIRE(name == "John Doe");
                }
                THEN("a request to the OCS endpoint should be made") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "http://nextcloud/ocs/v1.php/cloud/user");
                    REQUIRE_REQUEST(0, headers.at("OCS-APIRequest") == "true");
                    REQUIRE_REQUEST(0, headers.at("Accept") == Request::MIMETYPE_XML);
                }
            }
        }
        WHEN("calling get_base_url") {
            const std::string baseUrl = cloud->get_base_url();
            THEN("it should return the correct base url") {
                REQUIRE(baseUrl == "http://nextcloud");
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
            When(Method(requestMock, request)).Return(request::StringResponse(
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
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "DELETE");
                    REQUIRE_REQUEST(0, url == "http://nextcloud/ocs/v2.php/core/apppassword");
                    REQUIRE_REQUEST(0, headers.at("OCS-APIRequest") == "true");
                }
            }
        }
    }
}
