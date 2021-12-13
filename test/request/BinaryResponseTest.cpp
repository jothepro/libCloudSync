#include "request/BinaryResponse.hpp"
#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>

using namespace Catch;
using namespace CloudSync;
using json = nlohmann::json;
using namespace CloudSync::request;

SCENARIO("BinaryResponse", "[request]") {
    WHEN("constructing a BinaryResponse with code 404") {
        THEN("a NotFound exception should be thrown") {
            REQUIRE_THROWS_AS(BinaryResponse(404, {0x11, 0x12, 0x13}), exceptions::response::NotFound);
        }
    }
    WHEN("constructing a BinaryResponse with code 405") {
        THEN("a NotFound exception should be thrown") {
            REQUIRE_THROWS_AS(BinaryResponse(405, {0x11, 0x12, 0x13}), exceptions::response::MethodNotAllowed);
        }
        THEN("the body of the exception should hold the response content") {
            try {
                const std::string message = "unauthorized because foo";
                BinaryResponse(401, {message.begin(), message.end()}, "text/plain");
            } catch (const exceptions::response::Unauthorized& e) {
                REQUIRE(e.data == "unauthorized because foo");
            }
        }
    }
    WHEN("constructing a StringResponse with a json body") {
        const std::string message = json{{"hello", "world"}}.dump();
        THEN("calling .json() on the exception should parse the json response") {
            try {
                BinaryResponse(412, {message.begin(), message.end()}, "application/json");
            } catch (const exceptions::response::PreconditionFailed& e) {
                const auto response_json = e.json();
                REQUIRE(response_json.at("hello") == "world");
            }
        }
        THEN("calling .xml() on the exception should throw a ParseError exception") {
            try {
                BinaryResponse(503, {message.begin(), message.end()}, "application/json");
            } catch (const exceptions::response::ServiceUnavailable& e) {
                REQUIRE_THROWS_AS(e.xml(), exceptions::ParseError);
            }
        }
    }
}