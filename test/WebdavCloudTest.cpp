#include "webdav/WebdavCloud.hpp"
#include "macros/access_protected.hpp"
#include "macros/request_mock.hpp"
#include "macros/basic_auth_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;

SCENARIO("WebdavCloud", "[cloud][webdav]") {
    INIT_REQUEST();
    BASIC_AUTH_MOCK("john", "password123");
    GIVEN("a webdav cloud instance") {
        const auto cloud = std::make_shared<webdav::WebdavCloud>("http://cloud", credentials, request);
        WHEN("calling get_base_url") {
            const std::string base_url = cloud->get_base_url();
            THEN("it should return the correct base url") {
                REQUIRE(base_url == "http://cloud");
            }
        }
        WHEN("calling root()") {
            const auto directory = cloud->root();
            THEN("the root directory is returned") {
                REQUIRE(directory->name().empty());
                REQUIRE(directory->path() == "/");
            }
        }
    }
}
