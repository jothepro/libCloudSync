#include "MyLibrary/example.hpp"
#include <memory>

using namespace MyLibrary;

int main() {
    auto example = std::make_shared<Example>();
    example->test("test");
}
