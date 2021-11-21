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
using P = Request::ParameterType;

SCENARIO("GDriveCloud", "[cloud][gdrive]") {
    INIT_REQUEST();
    GIVEN("a google drive cloud instance") {
        const auto cloud = std::make_shared<gdrive::GDriveCloud>("root", request);
        AND_GIVEN("a request that returns a user description") {
            WHEN_REQUEST().RESPOND(request::Response(200, json{{"name", "john doe"}}.dump(), "application/json"));

            WHEN("calling getUserDisplayName()") {
                const auto name = cloud->getUserDisplayName();
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
            When(Method((requestMock), getCurrentAccessToken)).Return("mytoken");
            AND_GIVEN("a request that returns 200") {
                WHEN_REQUEST().RESPOND(request::Response(200));
                WHEN("calling logout()") {
                    cloud->logout();
                    THEN("the OAuth-token should be invalidated") {
                        REQUIRE_REQUEST_CALLED().Once();
                        REQUIRE_REQUEST(0, verb == "POST");
                        REQUIRE_REQUEST(0, url == "https://oauth2.googleapis.com/revoke");
                        REQUIRE_REQUEST(0, parameters.at(P::MIME_POSTFIELDS).at("token") == "mytoken");
                    }
                }
            }
        }
    }
}
