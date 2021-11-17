#include "box/BoxCloud.hpp"
#include "request/Request.hpp"
#include "macros/request_mock.hpp"
#include "macros/shared_ptr_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;

SCENARIO("BoxCloud", "[cloud][box]") {
    INIT_REQUEST()
    GIVEN("a box cloud instance") {
        const auto cloud = std::make_shared<box::BoxCloud>(request);
        AND_GIVEN("a request that returns a user account description") {
            WHEN_REQUEST().RESPOND(request::Response(200, json{{"name", "John Doe"}}.dump(), "application/json"));

            WHEN("calling getUserDisplayName()") {
                const auto name = cloud->getUserDisplayName();
                THEN("John Doe should be returned") {
                    REQUIRE(name == "John Doe");
                }
                THEN("the box users endpoint should be called") {
                    REQUIRE_REQUEST_CALLED().Once();
                    REQUIRE_REQUEST(0, verb == "GET");
                    REQUIRE_REQUEST(0, url == "https://api.box.com/2.0/users/me");
                }
            }
        }
        WHEN("calling root()") {
            const auto directory = cloud->root();
            THEN("the root directory is returned") {
                REQUIRE(directory->name == "");
                REQUIRE(directory->path == "/");
            }
        }
    }
}
