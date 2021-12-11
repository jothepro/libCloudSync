#include "gdrive/GDriveCloud.hpp"
#include "request/Request.hpp"
#include "macros/request_mock.hpp"
#include "macros/oauth_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;
using namespace CloudSync::request;

SCENARIO("GDriveCloud", "[cloud][gdrive]") {
    INIT_REQUEST();
    OAUTH_MOCK("mytoken");
    GIVEN("a google drive cloud instance") {
        const auto cloud = std::make_shared<gdrive::GDriveCloud>("root", credentials, request);
        AND_GIVEN("a request that returns a user description") {
            When(Method(requestMock, request)).Return(request::StringResponse(200, json{{"name", "john doe"}}.dump(), "application/json"));

            WHEN("calling get_user_display_name()") {
                const auto name = cloud->get_user_display_name();
                THEN("john doe should be returned") {
                    REQUIRE(name == "john doe");
                }
                THEN("a GET request to the google oauth2 userinfo endpoint should be made") {
                    Verify(Method(requestMock, request)).Once();
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
        AND_GIVEN("a request that returns 200") {
            When(Method(requestMock, request)).Return(request::StringResponse(200));
            WHEN("calling logout()") {
                cloud->logout();
                THEN("the OAuth-token should be invalidated") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://oauth2.googleapis.com/revoke");
                    REQUIRE_REQUEST(0, mime_postfields.at("token") == "mytoken");
                    REQUIRE_REQUEST(0, bearer_token == "mytoken");
                }
            }
        }
    }
}
