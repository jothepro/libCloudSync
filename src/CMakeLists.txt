cmake_minimum_required(VERSION 3.15)

project(MyLibrary)

# dependencies
find_package(nlohmann_json REQUIRED)

# library definition
add_library(MyLibrary
    example.cpp
    include/MyLibrary/example.hpp)
target_compile_features(MyLibrary PUBLIC cxx_std_17)
target_include_directories(MyLibrary
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(MyLibrary
    PUBLIC
        nlohmann_json::nlohmann_json
)

# library installation
install(TARGETS MyLibrary EXPORT MyLibraryTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)

install(DIRECTORY include/MyLibrary
        DESTINATION include)

add_library(MyLibrary::MyLibrary ALIAS MyLibrary)
