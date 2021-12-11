cmake_minimum_required(VERSION 3.15)

project(CloudSyncIntegrationTest)

find_package(cxxopts REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(CloudSyncIntegrationTest main.cpp)

target_link_libraries(CloudSyncIntegrationTest
    PRIVATE
        CloudSync::CloudSync
        cxxopts::cxxopts
        nlohmann_json::nlohmann_json
)

set_target_properties(CloudSyncIntegrationTest
    PROPERTIES
        EXCLUDE_FROM_ALL true
)