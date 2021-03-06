cmake_minimum_required(VERSION 3.15)

project(CloudSyncTest)

find_package(Catch2 MODULE REQUIRED)
find_package(fakeit MODULE REQUIRED)

include(Catch)

set(REQUEST_TEST_SRC
    request/StringResponseTest.cpp
    request/BinaryResponseTest.cpp)

source_group(request FILES ${REQUEST_TEST_SRC})

add_executable(CloudSyncTest
    main.cpp
    OAuth2CredentialsTest.cpp
    WebdavCloudTest.cpp
    NextcloudCloudTest.cpp
    WebdavDirectoryTest.cpp
    WebdavFileTest.cpp
    DropboxCloudTest.cpp
    DropboxDirectoryTest.cpp
    DropboxFileTest.cpp
    OneDriveCloudTest.cpp
    OneDriveDirectoryTest.cpp
    OneDriveFileTest.cpp
    GDriveCloudTest.cpp
    GDriveDirectoryTest.cpp
    GDriveFileTest.cpp
    CloudFactoryTest.cpp
    ${REQUEST_TEST_SRC}
)

target_link_libraries(CloudSyncTest
    Catch2::Catch2
    fakeit::fakeit
    CloudSync::CloudSync
)

set_target_properties (CloudSyncTest PROPERTIES
    FOLDER CloudSync
)

catch_discover_tests(CloudSyncTest)

target_compile_features(CloudSyncTest PRIVATE cxx_std_17)