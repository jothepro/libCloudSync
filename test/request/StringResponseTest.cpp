#include "request/StringResponse.hpp"
#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>

using namespace Catch;
using namespace CloudSync;
using json = nlohmann::json;
using namespace CloudSync::request;

SCENARIO("StringResponse", "[request]") {
    GIVEN("a StringResponse with code 200 and a valid json body") {
        const auto response = StringResponse(200, json{{"hello", "world"}}.dump(), "application/json");
        WHEN("calling .json()") {
            const auto json_content = response.json();
            THEN("the json content should be parsed") {
                REQUIRE(json_content.at("hello") == "world");
            }
        }
        WHEN("calling .xml()") {
            THEN("a ParseError exception should be thrown") {
                REQUIRE_THROWS_AS(response.xml(), exceptions::ParseError);
            }
        }
    }
    GIVEN("a StringResponse with code 200 and a valid xml body") {
        const auto response = StringResponse(200, "<?xml version=\"1.0\"?>\n<response>hello world</response>", "application/xml");
        WHEN("calling .xml()") {
            const auto xml_content = response.xml();
            THEN("the xml content should be parsed") {
                REQUIRE(std::string(xml_content->child("response").child_value()) == "hello world");
            }
        }
        WHEN("calling .json()") {
            THEN("a ParseError exception should be thrown") {
                REQUIRE_THROWS_AS(response.json(), exceptions::ParseError);
            }
        }
    }
    WHEN("constructing a Response with code 404") {
        THEN("a NotFound exception should be thrown") {
            REQUIRE_THROWS_AS(StringResponse(404, "not found"), exceptions::response::NotFound);
        }
    }
    WHEN("constructing a Response with code 412") {
        THEN("a NotFound exception should be thrown") {
            REQUIRE_THROWS_AS(StringResponse(412, "not found"), exceptions::response::PreconditionFailed);
        }
        THEN("the body of the exception should hold the response content") {
            try {
                StringResponse(409, "not found", "text/plain");
            } catch (const exceptions::response::Conflict& e) {
                REQUIRE(e.data == "not found");
            }
        }
    }
    WHEN("constructing a StringResponse with a json body") {
        THEN("calling .json() on the exception should parse the json response") {
            try {
                StringResponse(500, json{{"hello", "world"}}.dump(), "application/json");
            } catch (const exceptions::response::InternalServerError& e) {
                const auto response_json = e.json();
                REQUIRE(response_json.at("hello") == "world");
            }
        }
        THEN("calling .xml() on the exception should throw a ParseError exception") {
            try {
                StringResponse(403, json{{"hello", "world"}}.dump(), "application/json");
            } catch (const exceptions::response::Forbidden& e) {
                REQUIRE_THROWS_AS(e.xml(), exceptions::ParseError);
            }
        }
    }
}