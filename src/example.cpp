#include "MyLibrary/example.hpp"
#include <nlohmann/json.hpp>

using namespace MyLibrary;
using json = nlohmann::json;

std::string Example::test(const std::string& test) {
    json testJson = {{"hello", test}};
    return testJson.dump(4);
}