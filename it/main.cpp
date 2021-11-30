#include "CloudSync/CloudFactory.hpp"
#include "CloudSync/Directory.hpp"
#include "CloudSync/File.hpp"
#include "CloudSync/OAuth2Credentials.hpp"
#include "CloudSync/UsernamePasswordCredentials.hpp"
#include "macros/test.hpp"
#include <cxxopts.hpp>
#include <iomanip>
#include <iostream>
#include <vector>
#include <chrono>

int main(int argc, char *argv[]) {
    std::string providerUrl;
    std::shared_ptr<CloudSync::Cloud> cloud;
    std::shared_ptr<CloudSync::Directory> dir;
    std::string root = "root";

    cxxopts::Options options("CloudSyncIntegrationTest", "A test that runs against the real API, ensuring that the "
                                                         "library behaves as expected.");
    options.add_options()
            ("h,help", "Print help");
    options.add_options("1) cloud providers")
            ("webdav", "access webdav server")
            ("nextcloud", "access nextcloud server")
            ("owncloud", "access owncloud server")
            ("dropbox", "access dropbox cloud")
            ("box", "access box cloud")
            ("onedrive", "access onedrive server")
            ("gdrive", "access google drive");
    options.add_options("2) configuration")
            ("r,root", "root name (used in onedrive & gdrive)", cxxopts::value<std::string>())
            ("d,domain", "address under which the cloud can be found", cxxopts::value<std::string>());
    options.add_options("3) authentication")
            ("u,username", "username, if login with username & password should be used.", cxxopts::value<std::string>())
            ("p,password", "password, if login with username & password should be used.", cxxopts::value<std::string>())
            ("t,token", "OAuth2 access token.", cxxopts::value<std::string>());
    auto result = options.parse(argc, argv);

    // printing help
    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    // get server address
    if (result.count("domain")) {
        providerUrl = result["domain"].as<std::string>();
    } else {
        if(result.count("webdav") || result.count("nextcloud")) {
            std::cerr << "no domain provided" << std::endl;
            std::cout << options.help({"2) configuration"}) << std::endl;
            exit(1);
        }
    }

    if (result.count("root")) {
        root = result["root"].as<std::string>();
    }

    // find out cloud type
    if (result.count("webdav")) {
        cloud = CloudSync::CloudFactory().webdav(providerUrl);
    } else if (result.count("nextcloud")) {
        cloud = CloudSync::CloudFactory().nextcloud(providerUrl);
    } else if (result.count("dropbox")) {
        cloud = CloudSync::CloudFactory().dropbox();
    } else if (result.count("box")) {
        cloud = CloudSync::CloudFactory().box();
    } else if (result.count("onedrive")) {
        cloud = CloudSync::CloudFactory().onedrive(root);
    } else if (result.count("gdrive")) {
        cloud = CloudSync::CloudFactory().gdrive(root);
    } else {
        std::cerr << "no cloud provider specified" << std::endl;
        std::cout << options.help({"1) cloud providers"}) << std::endl;
        exit(1);
    }

    // find out login method and do login
    if (result.count("username") && result.count("password")) {
        auto credentials = CloudSync::UsernamePasswordCredentials(
            result["username"].as<std::string>(),
            result["password"].as<std::string>()
        );
        cloud->login(credentials);
    } else if (result.count("token")) {
        auto credentials = CloudSync::OAuth2Credentials(result["token"].as<std::string>());
        cloud->login(credentials);
    } else {
        std::cerr << "no username/password or token provided" << std::endl;
        std::cout << options.help({"3) authentication"}) << std::endl;
        exit(1);
    }

    const auto test_start_time = std::chrono::system_clock::now();

    TEST_IF("Getting user display name",
        auto user_display_name = cloud->get_user_display_name(),
        (!user_display_name.empty())
    )

    std::cout << user_display_name << std::endl;

    TEST_IF("Getting root()",
        auto rootDir = cloud->root(),
        (rootDir->name().empty() && rootDir->path() == "/")
    )

    TEST_THROWS("When calling remove on the root dir, a PermissionDenied exception should be thrown",
        rootDir->remove(),
        CloudSync::Resource::PermissionDenied);

    const auto test_directory_name = "test_" + std::to_string(test_start_time.time_since_epoch().count());
    TEST_IF("Creating a directory " + test_directory_name,
        auto test_directory = rootDir->create_directory(test_directory_name),
        (test_directory->name() == test_directory_name && test_directory->path() == "/" + test_directory_name)
    )

    TEST_IF("When calling list_resources() on the (empty) directory, an empty resource list should be returned",
        auto testDirectoryResources = test_directory->list_resources(),
        (testDirectoryResources.empty())
    )

    TEST_IF("When calling create_file(test.txt), the (empty) file should be returned",
        auto test_file = test_directory->create_file("test.txt"),
        (test_file->name() == "test.txt" && test_file->path() == "/" + test_directory_name + "/test.txt")
    )

    TEST_IF("When reading the content of the new file, an emtpy string should be returned",
        auto test_file_content = test_file->read_as_string(),
        (test_file_content.empty())
    )

    TEST("Writing content to the file 'test.txt'",
        test_file->write_string("hello from test")
    )

    TEST_IF("Reading content from the file 'test.txt'",
        auto new_test_file_content = test_file->read_as_string(),
        (new_test_file_content == "hello from test")
    )

    TEST_THROWS("When creating another file with the same name 'test.txt', a ResourceConflict exception should be thrown",
        test_directory->create_file("test.txt"), CloudSync::Resource::ResourceConflict
    )

    TEST_THROWS("When creating an folder with the name 'test.txt', a ResourceConflict exception should be thrown",
        test_directory->create_directory("test.txt"), CloudSync::Resource::ResourceConflict
    )

    TEST_THROWS("When calling get_directory(test.txt), a NoSuchResource exception should be thrown",
        test_directory->get_directory("test.txt"), CloudSync::Resource::NoSuchResource
    )

    TEST("Getting a new reference to the same 'test.txt' file",
        auto test_file_2 = test_directory->get_file("test.txt")
    )

    TEST("Writing new file content to the new reference to 'test.txt'",
        test_file_2->write_string("new content")
    )

    TEST_THROWS("When trying to write new content from the original (first) reference to 'test.txt', "
                "a ResourceHasChanged exception should be thrown",
        test_file->write_string("even newer content"), CloudSync::Resource::ResourceHasChanged
    )

    TEST_IF("When calling poll_change(), the revision on the old reference should be updated",
             auto poll_successful = test_file->poll_change(),
             (test_file_2->revision() == test_file->revision() && poll_successful)
    )

    TEST_IF("When calling poll_change() again, the poll should return false",
            auto poll_successful_2 = test_file->poll_change(),
            (!poll_successful_2)
    )

    TEST_IF("When trying to write new content from the original (first) reference to 'test.txt' again, "
            "then it should now succeed",
        test_file->write_string("even newer content"),
        (test_file->revision() != test_file_2->revision())
    )

    TEST_IF("When calling create_directory(new_directory), then a new directory should be returned",
        auto new_directory = test_directory->create_directory("new_directory"),
        (new_directory->name() == "new_directory" && new_directory->path() == "/" + test_directory_name + "/new_directory")
    )

    TEST_THROWS("When creating another folder with the same name 'new_directory', a ResourceConflict exception should be thrown",
        test_directory->create_directory("new_directory"), CloudSync::Resource::ResourceConflict
    )

    TEST_THROWS("When creating a file with the name 'new_directory', a ResourceConflict exception should be thrown",
        test_directory->create_file("new_directory"), CloudSync::Resource::ResourceConflict
    )

    TEST_THROWS("When calling get_file(new_directory), a NoSuchResourceException should be thrown",
        test_directory->get_file("new_directory"), CloudSync::Resource::NoSuchResource
    )

    TEST_IF("When calling list_resources(), then a resource list with both 'test.txt' and 'new_directory' "
            "should be returned in alphabetical order",
        auto test_directory_resources_2 = test_directory->list_resources(),
        (
            test_directory_resources_2.size() == 2 &&
            test_directory_resources_2[0]->name() == "new_directory" &&
            test_directory_resources_2[1]->name() == "test.txt"
        )
    )

    TEST_IF("When calling create_directory('new_directory/test'), a new directory 'test' should be created inside the "
        "existing 'new_directory' directory",
        auto new_directory_test_directory = test_directory->create_directory("new_directory/test"),
        (new_directory_test_directory->name() == "test" && new_directory_test_directory->path() == "/" + test_directory_name + "/new_directory/test")
    )

    TEST_IF("When calling create_directory('test/test'), a new directory 'test' should be created inside a new directory 'test'",
        auto test_test_directory = test_directory->create_directory("test/test"),
        (test_test_directory->name() == "test" && test_test_directory->path() == "/" + test_directory_name + "/test/test")
    )

    TEST_IF("When calling create_file('new_directory/test.txt'), a new file 'test.txt' should be created inside the "
            "existing 'new_directory' directory",
        auto new_directory_test_file = test_directory->create_file("new_directory/test.txt"),
        (new_directory_test_file->name() == "test.txt" && new_directory_test_file->path() == "/" + test_directory_name + "/new_directory/test.txt")
    )

    TEST_IF("When calling create_file('test2/test2.txt'), a new file 'test.txt' should be created inside a newly created "
            "'test2' directory",
        auto test2_test2_file = test_directory->create_file("test2/test2.txt"),
            (test2_test2_file->name() == "test2.txt" && test2_test2_file->path() == "/" + test_directory_name + "/test2/test2.txt")
    )

    TEST("Deleting the 'test/test' directory",
        test_test_directory->remove()
    )

    TEST("Deleting the 'test' directory",
         test_directory->get_directory("test")->remove()
    )

    TEST("Deleting the 'new_directory/test' directory",
         new_directory_test_directory->remove()
    )

    TEST("Deleting the 'new_directory/test.txt' file",
        new_directory_test_file->remove()
    )

    TEST("Deleting the 'test2/test2.txt' file",
         test2_test2_file->remove()
    )

    TEST("Deleting the 'test2' directory",
         test_directory->get_directory("test2")->remove()
    )

    TEST("Deleting the directory 'new_directory'",
        new_directory->remove()
    )

    TEST("Deleting the file 'test.txt'",
        test_file->remove()
    )

    TEST("Deleting the test directory",
         test_directory->remove());

    const auto test_end_time = std::chrono::system_clock::now();

    std::cout << "\n=============\nTest duration: " << std::chrono::duration_cast<std::chrono::seconds>(test_end_time - test_start_time).count() << "s" << std::endl;

    return 0;
}