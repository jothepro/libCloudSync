cmake_minimum_required(VERSION 3.15)
# Appends the cmake/modules path to MAKE_MODULE_PATH variable.
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

include(GetVersion)
include(${CMAKE_BINARY_DIR}/conan_paths.cmake OPTIONAL)

get_version(PROJECT_VERSION)

project(CloudSync
    LANGUAGES CXX
    VERSION ${PROJECT_VERSION}
)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target( doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif(DOXYGEN_FOUND)

add_subdirectory(lib)
add_subdirectory(example)

include(CTest)
if(BUILD_TESTING)
    set(CMAKE_CTEST_ARGUMENTS "--verbose")
    add_subdirectory(test)
endif()
