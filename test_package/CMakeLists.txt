cmake_minimum_required(VERSION 3.15)
include(${CMAKE_BINARY_DIR}/conan_paths.cmake OPTIONAL)
project(PackageTest 
    LANGUAGES CXX)
find_package(CloudSync)
add_executable(PackageTest example.cpp)
target_link_libraries(PackageTest CloudSync::CloudSync)
target_compile_features(PackageTest PRIVATE cxx_std_17)
set_target_properties(PackageTest
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin"
)