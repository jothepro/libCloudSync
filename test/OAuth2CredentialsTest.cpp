
#include "credentials/OAuth2CredentialsImpl.hpp"
#include "request/Request.hpp"
#include "macros/request_mock.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <nlohmann/json.hpp>
#include <chrono>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;
using namespace CloudSync::request;
using namespace CloudSync::credentials;
using json = nlohmann::json;
using namespace std::chrono_literals;

SCENARIO("OAuth2Credentials", "[credentials]") {
    INIT_REQUEST();
    const std::string TOKEN_ENDPOINT = "https://cloud/token";
    GIVEN("an OAuth2Credentials instance created from_access_token() with a token that does not expire") {
        const auto credentials = std::static_pointer_cast<OAuth2CredentialsImpl>(OAuth2Credentials::from_access_token("4lcoefgiateho_token"));

        WHEN("fetching the token") {
            const auto token = credentials->get_current_access_token();
            THEN("the current token should be returned") {
                REQUIRE(token == "4lcoefgiateho_token");
            }
        }
    }
    GIVEN("an OAuth2Credentials instance created from_refresh_token()") {
        const auto credentials = std::static_pointer_cast<OAuth2CredentialsImpl>(OAuth2Credentials::from_refresh_token(
                "123-client-id",
                "llcw5x_refresh_token"));
        credentials->set_request(request);
        credentials->set_token_endpoint(TOKEN_ENDPOINT);
        AND_GIVEN("a request that return a new access_token") {
            When(Method(requestMock, request)).Return(request::StringResponse(200, json{
                    {"access_token", "sl.43wtghareor_new_token"},
                    {"token_type", "bearer"},
                    {"expires_in", 3600}
            }.dump()));
            AND_GIVEN("a callback being registered for a token update") {
                std::string new_access_token;
                std::chrono::system_clock::time_point new_token_expires;
                std::string new_refresh_token;

                credentials->on_token_update([&new_access_token, &new_token_expires, &new_refresh_token](
                        const std::string& access_token,
                        std::chrono::system_clock::time_point expires,
                        const std::string& refresh_token){
                    new_access_token = access_token;
                    new_token_expires = expires;
                    new_refresh_token = refresh_token;
                });

                WHEN("fetching the token") {
                    const auto token = credentials->get_current_access_token();
                    THEN("a new refreshed access-token should be returned") {
                        REQUIRE(token == "sl.43wtghareor_new_token");
                    }
                    THEN("a POST request should have been made to the token endpoint to ask for a new token") {
                        Verify(Method(requestMock, request)).Once();
                        REQUIRE_REQUEST(0, url == TOKEN_ENDPOINT);
                        REQUIRE_REQUEST(0, verb == "POST");
                        REQUIRE_REQUEST(0, postfields.at("client_id") == "123-client-id");
                        REQUIRE_REQUEST(0, postfields.at("grant_type") == "refresh_token");
                        REQUIRE_REQUEST(0, postfields.at("refresh_token") == "llcw5x_refresh_token");
                    }
                    THEN("the on_token_update callback should have been triggered with the new token") {
                        REQUIRE(new_access_token == "sl.43wtghareor_new_token");
                        REQUIRE(new_token_expires > std::chrono::system_clock::now() + 3590s);
                        REQUIRE(new_token_expires < std::chrono::system_clock::now() + 3610s);
                        REQUIRE(new_refresh_token == "llcw5x_refresh_token");
                    }
                }
            }

        }
    }
    AND_GIVEN("an OAuth2Credentials instance created from_authorization_code()") {
        const std::string CLIENT_ID = "123-client-id";
        const std::string AUTHORIZATION_CODE = "fggoh34rh_authorization_code";
        const std::string REDIRECT_URI = "http://localhost:4562/login";
        const std::string CODE_VERIFIER = "vfgetaehrp6_code_verifier";
        const auto credentials = std::static_pointer_cast<OAuth2CredentialsImpl>(OAuth2Credentials::from_authorization_code(
                CLIENT_ID,
                AUTHORIZATION_CODE,
                REDIRECT_URI,
                CODE_VERIFIER));
        credentials->set_request(request);
        credentials->set_token_endpoint(TOKEN_ENDPOINT);
        AND_GIVEN("a request that returns a new access_token & refresh token") {
            const std::string NEW_ACCESS_TOKEN = "sl.goghf5492_access_token";
            const std::string NEW_REFRESH_TOKEN = "icwsnftf08e_refresh_token";
            When(Method(requestMock, request)).Return(request::StringResponse(200, json{
                    {"access_token", NEW_ACCESS_TOKEN},
                    {"expires_in", 3600},
                    {"refresh_token", NEW_REFRESH_TOKEN}
            }.dump()));

            AND_GIVEN("a callback being registered for token updates") {
                std::string new_access_token;
                std::chrono::system_clock::time_point new_token_expires;
                std::string new_refresh_token;

                credentials->on_token_update([&new_access_token, &new_token_expires, &new_refresh_token](
                        const std::string& access_token,
                        std::chrono::system_clock::time_point expires,
                        const std::string& refresh_token){
                    new_access_token = access_token;
                    new_token_expires = expires;
                    new_refresh_token = refresh_token;
                });

                WHEN("fetching the access-token") {
                    const auto token = credentials->get_current_access_token();
                    THEN("a new access-token should be returned") {
                        REQUIRE(token == NEW_ACCESS_TOKEN);
                    }
                    THEN("a POST request should be made to the token endpoint") {
                        Verify(Method(requestMock, request)).Once();
                        REQUIRE_REQUEST(0, url == TOKEN_ENDPOINT);
                        REQUIRE_REQUEST(0, verb == "POST");
                        REQUIRE_REQUEST(0, postfields.at("grant_type") == "authorization_code");
                        REQUIRE_REQUEST(0, postfields.at("code") == AUTHORIZATION_CODE);
                        REQUIRE_REQUEST(0, postfields.at("redirect_uri") == REDIRECT_URI);
                        REQUIRE_REQUEST(0, postfields.at("code_verifier") == CODE_VERIFIER);
                        REQUIRE_REQUEST(0, postfields.at("client_id") == CLIENT_ID);
                    }
                    THEN("the on_token_update() callback should be called with the new access-token & refresh-token") {
                        REQUIRE(new_access_token == NEW_ACCESS_TOKEN);
                        REQUIRE(new_token_expires > std::chrono::system_clock::now() + 3590s);
                        REQUIRE(new_token_expires < std::chrono::system_clock::now() + 3610s);
                        REQUIRE(new_refresh_token == NEW_REFRESH_TOKEN);
                    }
                }
            }
        }
    }
}