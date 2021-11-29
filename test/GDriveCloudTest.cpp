#include "gdrive/GDriveCloud.hpp"
#include "request/Request.hpp"
#include "macros/request_mock.hpp"
#include "macros/shared_ptr_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;
using namespace CloudSync::request;

SCENARIO("GDriveCloud", "[cloud][gdrive]") {
    INIT_REQUEST();
    GIVEN("a google drive cloud instance") {
        const auto cloud = std::make_shared<gdrive::GDriveCloud>("root", request);
        AND_GIVEN("a request that returns a user description") {
            WHEN_REQUEST().RESPOND(request::Response(200, json{{"name", "john doe"}}.dump(), "application/json"));

            WHEN("calling get_user_display_name()") {
                const auto name = cloud->get_user_display_name();
                THEN("john doe should be returned") {
                    REQUIRE(name == "john doe");
                }
                THEN("a request the google oauth2 userinfo endpoint should be made") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "https://www.googleapis.com/userinfo/v2/me");
                }
            }
        }
        WHEN("calling root()") {
            const auto directory = cloud->root();
            THEN("the root directory is returned") {
                REQUIRE(directory->name() == "");
                REQUIRE(directory->path() == "/");
            }
        }
        AND_GIVEN("the currently used access-token being 'mytoken'") {
            When(Method(requestMock, get_current_access_token)).Return("mytoken");
            AND_GIVEN("a request that returns 200") {
                WHEN_REQUEST().RESPOND(request::Response(200));
                WHEN("calling logout()") {
                    When(Method(requestMock, reset_auth)).Return();
                    cloud->logout();
                    THEN("the OAuth-token should be invalidated") {
                        REQUIRE_REQUEST_CALLED().Once();
                        REQUIRE_REQUEST(0, verb == "POST");
                        REQUIRE_REQUEST(0, url == "https://oauth2.googleapis.com/revoke");
                        REQUIRE_REQUEST(0, mime_postfields.at("token") == "mytoken");
                    }
                    THEN("the request credentials should be reset") {
                        Verify(Method(requestMock, reset_auth)).Once();
                    }
                }
            }
        }
    }
}
