#include "CloudSync/CloudFactory.hpp"
#include "request/Request.hpp"
#include "box/BoxCloud.hpp"
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
    GIVEN("a cloudFactory instance") {
        auto cloudFactory = std::make_shared<CloudFactory>();
        WHEN("calling dropbox()") {
            const auto cloud = cloudFactory->dropbox();
            THEN("a dropbox cloud instance should be returned") {
                REQUIRE(std::dynamic_pointer_cast<dropbox::DropboxCloud>(cloud));
            }
        }
        WHEN("calling box()") {
            const auto cloud = cloudFactory->box();
            THEN("a box cloud instance should be returned") {
                REQUIRE(std::dynamic_pointer_cast<box::BoxCloud>(cloud));
            }
        }
        WHEN("calling nextcloud(\"my.cloud\")") {
            const auto cloud = cloudFactory->nextcloud("my.cloud");
            THEN("a nextcloud cloud instance should be returned") {
                REQUIRE(std::dynamic_pointer_cast<nextcloud::NextcloudCloud>(cloud));
            }
            THEN("the nextcloud instance should hold the correct url") {
                REQUIRE(cloud->getBaseUrl() == "my.cloud");
            }
        }
        WHEN("calling onedrive()") {
            const auto cloud = cloudFactory->onedrive();
            THEN("a onedrive cloud instance should be returned") {
                REQUIRE(std::dynamic_pointer_cast<onedrive::OneDriveCloud>(cloud));
            }
        }
        WHEN("calling gdrive()") {
            const auto cloud = cloudFactory->gdrive();
            THEN("a gdrive cloud instance should be returned") {
                REQUIRE(std::dynamic_pointer_cast<gdrive::GDriveCloud>(cloud));
            }
        }
        WHEN("calling webdav(\"my.cloud\")") {
            const auto cloud = cloudFactory->webdav("my.cloud");
            THEN("a webdav cloud instance should be returned") {
                REQUIRE(std::dynamic_pointer_cast<webdav::WebdavCloud>(cloud));
            }
            THEN("the webdav instance should hold the correct url") {
                REQUIRE(cloud->getBaseUrl() == "my.cloud");
            }
        }
    }
}
