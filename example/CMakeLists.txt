cmake_minimum_required(VERSION 3.15)

project(CloudSyncExample)

find_package(cxxopts REQUIRED)

add_executable(CloudSyncExample main.cpp)

target_link_libraries(CloudSyncExample
    PRIVATE
        CloudSync::CloudSync
        cxxopts::cxxopts
)

set_target_properties(CloudSyncExample
    PROPERTIES
        EXCLUDE_FROM_ALL true
)