#include "dropbox/DropboxCloud.hpp"
#include "request/Request.hpp"
#include "macros/request_mock.hpp"
#include "macros/shared_ptr_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;

SCENARIO("DropboxCloud", "[cloud][dropbox]") {
    INIT_REQUEST();
    GIVEN("a dropbox cloud instance") {
        const auto cloud = std::make_shared<dropbox::DropboxCloud>(request);
        AND_GIVEN("a request that returns the users account information") {
            WHEN_REQUEST().RESPOND(
                request::Response(200, json{{"name", {{"display_name", "John Doe"}}}}.dump(), "application/json"));
            WHEN("calling get_user_display_name()") {
                const auto name = cloud->get_user_display_name();
                THEN("John Doe should be returned") {
                    REQUIRE(name == "John Doe");
                }
                THEN("a request to get_current_account should be made") {
                    REQUIRE_REQUEST_CALLED().Once();
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
            WHEN_REQUEST().RESPOND(request::Response(200));
            WHEN("calling logout()") {
                When(Method(requestMock, reset_auth)).Return();
                cloud->logout();
                THEN("the OAuth-token should be invalidated") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "POST");
                    REQUIRE_REQUEST(0, url == "https://api.dropboxapi.com/2/auth/token/revoke");
                }
                THEN("the request credentials should be reset") {
                    Verify(Method(requestMock,reset_auth)).Once();
                }
            }
        }
    }
}
