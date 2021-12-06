cmake_minimum_required(VERSION 3.15)

project(CloudSync)

# dependencies
find_package(nlohmann_json REQUIRED)
find_package(pugixml REQUIRED)
find_package(CURL REQUIRED)

set(INCLUDE
    include/CloudSync/CloudFactory.hpp
    include/CloudSync/Cloud.hpp
    include/CloudSync/Directory.hpp
    include/CloudSync/File.hpp
    include/CloudSync/Resource.hpp
    include/CloudSync/OAuth2Credentials.hpp
    include/CloudSync/BasicCredentials.hpp
)

set(INCLUDE_EXCEPTIONS
    include/CloudSync/exceptions/Exception.hpp
)

set(INCLUDE_EXCEPTIONS_CLOUD
    include/CloudSync/exceptions/cloud/CloudException.hpp
)

set(INCLUDE_EXCEPTIONS_RESOURCE
    include/CloudSync/exceptions/resource/ResourceException.hpp
)

set(SRC
    src/CloudFactory.cpp
    src/CloudImpl.hpp
    src/CloudImpl.cpp
    src/OAuthCloudImpl.hpp
    src/OAuth2Credentials.cpp
    src/BasicCredentials.cpp
    src/DirectoryImpl.hpp
    src/DirectoryImpl.cpp
    src/OAuthDirectoryImpl.hpp
    src/FileImpl.cpp
    src/FileImpl.hpp
    src/OAuthFileImpl.hpp
)

set(SRC_CREDENTIALS
    src/credentials/BasicCredentialsImpl.hpp
    src/credentials/BasicCredentialsImpl.cpp
    src/credentials/OAuth2CredentialsImpl.cpp
    src/credentials/OAuth2CredentialsImpl.hpp
)

set(SRC_WEBDAV
    src/webdav/WebdavCloud.cpp
    src/webdav/WebdavCloud.hpp
    src/webdav/WebdavDirectory.hpp
    src/webdav/WebdavDirectory.cpp
    src/webdav/WebdavFile.hpp
    src/webdav/WebdavFile.cpp
)

set(SRC_NEXTCLOUD
    src/nextcloud/NextcloudCloud.cpp
    src/nextcloud/NextcloudCloud.hpp
)

set(SRC_DROPBOX
    src/dropbox/DropboxCloud.cpp
    src/dropbox/DropboxCloud.hpp
    src/dropbox/DropboxDirectory.hpp
    src/dropbox/DropboxDirectory.cpp
    src/dropbox/DropboxFile.hpp
    src/dropbox/DropboxFile.cpp
)

set(SRC_ONEDRIVE
    src/onedrive/OneDriveCloud.cpp
    src/onedrive/OneDriveCloud.hpp
    src/onedrive/OneDriveDirectory.hpp
    src/onedrive/OneDriveDirectory.cpp
    src/onedrive/OneDriveFile.hpp
    src/onedrive/OneDriveFile.cpp
)

set(SRC_GDRIVE
    src/gdrive/GDriveCloud.cpp
    src/gdrive/GDriveCloud.hpp
    src/gdrive/GDriveDirectory.hpp
    src/gdrive/GDriveDirectory.cpp
    src/gdrive/GDriveFile.hpp
    src/gdrive/GDriveFile.cpp
)

set(SRC_REQUEST
    src/request/Request.hpp
    src/request/Response.hpp
    src/request/Request.cpp
)

set(SRC_CURL_REQUEST
    src/request/curl/CurlRequest.cpp
    src/request/curl/CurlRequest.hpp
)

source_group(request\\curl FILES ${SRC_REQUEST_CURL})
source_group(include\\CloudSync FILES ${INCLUDE})
source_group(include\\CloudSync\\exceptions FILES ${INCLUDE_EXCEPTIONS})
source_group(include\\CloudSync\\exceptions\\resource FILES ${INCLUDE_EXCEPTIONS_RESOURCE})
source_group(include\\CloudSync\\exceptions\\cloud FILES ${INCLUDE_EXCEPTIONS_CLOUD})
source_group(src\\ FILES ${SRC})
source_group(src\\credentials FILES ${SRC_CREDENTIALS})
source_group(src\\webdav FILES ${SRC_WEBDAV})
source_group(src\\nextcloud FILES ${SRC_NEXTCLOUD})
source_group(src\\dropbox FILES ${SRC_DROPBOX})
source_group(src\\onedrive FILES ${SRC_ONEDRIVE})
source_group(src\\gdrive FILES ${SRC_GDRIVE})
source_group(src\\request FILES ${SRC_REQUEST})
source_group(src\\request\\curl FILES ${SRC_CURL_REQUEST})

# library definition
add_library(CloudSync
    ${INCLUDE}
    ${INCLUDE_EXCEPTIONS}
    ${INCLUDE_EXCEPTIONS_RESOURCE}
    ${INCLUDE_EXCEPTIONS_CLOUD}
    ${SRC}
    ${SRC_CREDENTIALS}
    ${SRC_WEBDAV}
    ${SRC_NEXTCLOUD}
    ${SRC_DROPBOX}
    ${SRC_ONEDRIVE}
    ${SRC_GDRIVE}
    ${SRC_REQUEST}
    ${SRC_CURL_REQUEST}
)
target_compile_features(CloudSync PUBLIC cxx_std_17)
target_include_directories(CloudSync
    PUBLIC
        include
        src
)

target_link_libraries(CloudSync
    PUBLIC
        nlohmann_json::nlohmann_json
        pugixml::pugixml
        CURL::CURL
)

set_target_properties (CloudSync PROPERTIES
    FOLDER CloudSync
)

# library installation
install(TARGETS CloudSync EXPORT CloudSyncTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)

install(DIRECTORY include/CloudSync
        DESTINATION include)

add_library(CloudSync::CloudSync ALIAS CloudSync)
