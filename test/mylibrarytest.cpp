#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <memory>
#include "MyLibrary/example.hpp"

using namespace MyLibrary;

SCENARIO("MyLibrary testing", "[test]") {
    GIVEN("an Example instance") {
        auto example = std::make_shared<Example>();

        WHEN("calling test()") {
            auto result = example->test("test");
            THEN("result should contain a test json") {
                REQUIRE(!result.empty());
            }
        }
    }
}