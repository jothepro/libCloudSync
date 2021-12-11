#include "dropbox/DropboxCloud.hpp"
#include "request/Request.hpp"
#include "macros/request_mock.hpp"
#include "macros/shared_ptr_mock.hpp"
#include "macros/oauth_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;

SCENARIO("DropboxCloud", "[cloud][dropbox]") {
    INIT_REQUEST();
    OAUTH_MOCK("mytoken");
    GIVEN("a dropbox cloud instance") {
        const auto cloud = std::make_shared<dropbox::DropboxCloud>(credentials, request);
        AND_GIVEN("a request that returns the users account information") {
            When(Method(requestMock, request))
                .Return(request::StringResponse(200, json{{"name", {{"display_name", "John Doe"}}}}.dump(), "application/json"));
            WHEN("calling get_user_display_name()") {
                const auto name = cloud->get_user_display_name();
                THEN("John Doe should be returned") {
                    REQUIRE(name == "John Doe");
                }
                THEN("a request to get_current_account should be made") {
                    Verify(Method(requestMock, request)).Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/users/get_current_account");
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
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/auth/token/revoke");
                }
            }
        }
    }
}
