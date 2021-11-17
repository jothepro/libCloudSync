#include "webdav/WebdavCloud.hpp"
#include "CloudSync/Credentials.hpp"
#include "CloudSync/Proxy.hpp"
#include "macros/access_protected.hpp"
#include "macros/request_mock.hpp"
#include "macros/shared_ptr_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;

SCENARIO("WebdavCloud", "[cloud][webdav]") {
    INIT_REQUEST();
    GIVEN("a webdav cloud instance") {
        const auto cloud = std::make_shared<webdav::WebdavCloud>("http://cloud", request);
        WHEN("calling getBaseUrl") {
            const auto baseUrl = cloud->getBaseUrl();
            THEN("it should return the correct base url") {
                REQUIRE(baseUrl == "http://cloud");
            }
        }
        WHEN("calling getTokenUrl") {
            const std::string result = cloud->getTokenUrl();
            THEN("an empty string should be returned") {
                REQUIRE(result.empty());
            }
        }
        WHEN("calling getAuthorizeUrl") {
            const std::string result = cloud->getAuthorizeUrl();
            THEN("an empty string should be returned") {
                REQUIRE(result.empty());
            }
        }
        WHEN("setting the proxy") {
            When(Method((*requestMock), setProxy)).Return();
            ACCESS_PROTECTED((CloudSync::Proxy), apply);
            auto proxyMock = Mock<apply_struct>();
            When(Method(proxyMock, apply)).Return();
            const auto returnedProxy = cloud->proxy(proxyMock.get());
            THEN("apply should be called on the proxyMock") {
                Verify(Method(proxyMock, apply)).Once();
            }
        }
        WHEN("setting the credentials") {
            When(Method((*requestMock), setTokenRequestUrl)).Return();
            ACCESS_PROTECTED((CloudSync::Credentials), apply);
            auto credentialsMock = Mock<apply_struct>();
            When(Method(credentialsMock, apply)).Return();
            const auto returnedCredentials = cloud->login(credentialsMock.get());
            THEN("apply should be called on the credentialsMock") {
                Verify(Method(credentialsMock, apply)).Once();
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
