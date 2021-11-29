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
    include/CloudSync/Exceptions.hpp
    include/CloudSync/File.hpp
    include/CloudSync/Resource.hpp
    include/CloudSync/Credentials.hpp
    include/CloudSync/OAuth2Credentials.hpp
    include/CloudSync/UsernamePasswordCredentials.hpp
)

set(SRC
    src/CloudFactory.cpp
    src/CloudImpl.hpp
    src/CloudImpl.cpp
    src/OAuth2Credentials.cpp
    src/UsernamePasswordCredentials.cpp
    src/DirectoryImpl.hpp
    src/DirectoryImpl.cpp
    src/FileImpl.cpp
    src/FileImpl.hpp
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

set(SRC_BOX
    src/box/BoxCloud.cpp
    src/box/BoxCloud.hpp
    src/box/BoxDirectory.hpp
    src/box/BoxDirectory.cpp
    src/box/BoxFile.hpp
    src/box/BoxFile.cpp
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

source_group(request\\curl FILES ${SRC_REQUEST_CURL})
source_group(include\\CloudSync FILES ${INCLUDE})
source_group(src\\ FILES ${SRC})
source_group(src\\webdav FILES ${SRC_WEBDAV})
source_group(src\\nextcloud FILES ${SRC_NEXTCLOUD})
source_group(src\\dropbox FILES ${SRC_DROPBOX})
source_group(src\\box FILES ${SRC_BOX})
source_group(src\\onedrive FILES ${SRC_ONEDRIVE})
source_group(src\\gdrive FILES ${SRC_GDRIVE})
source_group(src\\request FILES ${SRC_REQUEST})

# library definition
add_library(CloudSync
    ${INCLUDE}
    ${SRC}
    ${SRC_WEBDAV}
    ${SRC_NEXTCLOUD}
    ${SRC_DROPBOX}
    ${SRC_BOX}
    ${SRC_ONEDRIVE}
    ${SRC_GDRIVE}
    ${SRC_REQUEST}
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
