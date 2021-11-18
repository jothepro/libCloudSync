#define CATCH_CONFIG_RUNNER // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

int main(int argc, char *argv[]) {
    return Catch::Session().run(argc, argv);
}
