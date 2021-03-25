cmake_minimum_required(VERSION 3.15)
include(${CMAKE_BINARY_DIR}/conan_paths.cmake OPTIONAL)
project(PackageTest 
    LANGUAGES CXX)
find_package(MyLibrary REQUIRED)
add_executable(example example.cpp)
target_link_libraries(example MyLibrary::MyLibrary)
target_compile_features(example PRIVATE cxx_std_17)
set_target_properties(example
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFOE "${CMAKE_BINARY_DIR}/bin"
)