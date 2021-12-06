#include "CloudSync/CloudFactory.hpp"
#include "request/Request.hpp"
#include "dropbox/DropboxCloud.hpp"
#include "gdrive/GDriveCloud.hpp"
#include "macros/request_mock.hpp"
#include "macros/shared_ptr_mock.hpp"
#include "nextcloud/NextcloudCloud.hpp"
#include "onedrive/OneDriveCloud.hpp"
#include "webdav/WebdavCloud.hpp"
#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <memory>

using namespace fakeit;
using namespace Catch;
using namespace CloudSync;

SCENARIO("CloudFactory", "[cloud][factory]") {
    INIT_REQUEST();
    SHARED_PTR_MOCK(oauth_credentials, credentials::OAuth2CredentialsImpl);
    SHARED_PTR_MOCK(basic_credentials, credentials::BasicCredentialsImpl);
    GIVEN("a cloudFactory instance") {
        auto cloudFactory = std::make_shared<CloudFactory>();
        WHEN("calling create_dropbox()") {
            const auto cloud = cloudFactory->create_dropbox(oauth_credentials);
            THEN("a dropbox cloud instance should be returned") {
                REQUIRE(std::dynamic_pointer_cast<dropbox::DropboxCloud>(cloud));
            }
        }
        WHEN("calling create_nextcloud(\"my.cloud\")") {
            const auto cloud = cloudFactory->create_nextcloud("my.cloud", basic_credentials);
            THEN("a nextcloud cloud instance should be returned") {
                REQUIRE(std::dynamic_pointer_cast<nextcloud::NextcloudCloud>(cloud));
            }
            THEN("the nextcloud instance should hold the correct url") {
                REQUIRE(cloud->get_base_url() == "my.cloud");
            }
        }
        WHEN("calling create_onedrive()") {
            const auto cloud = cloudFactory->create_onedrive(oauth_credentials);
            THEN("a create_onedrive cloud instance should be returned") {
                REQUIRE(std::dynamic_pointer_cast<onedrive::OneDriveCloud>(cloud));
            }
        }
        WHEN("calling create_gdrive()") {
            const auto cloud = cloudFactory->create_gdrive(oauth_credentials);
            THEN("a gdrive cloud instance should be returned") {
                REQUIRE(std::dynamic_pointer_cast<gdrive::GDriveCloud>(cloud));
            }
        }
        WHEN("calling create_webdav(\"my.cloud\")") {
            const auto cloud = cloudFactory->create_webdav("my.cloud", basic_credentials);
            THEN("a webdav cloud instance should be returned") {
                REQUIRE(std::dynamic_pointer_cast<webdav::WebdavCloud>(cloud));
            }
            THEN("the webdav instance should hold the correct url") {
                REQUIRE(cloud->get_base_url() == "my.cloud");
            }
        }
    }
}
