#include "CloudSync/CloudFactory.hpp"
#include "CloudSync/Directory.hpp"
#include "CloudSync/File.hpp"
#include "CloudSync/OAuth2Credentials.hpp"
#include "CloudSync/exceptions/Exception.hpp"
#include <cxxopts.hpp>
#include <iomanip>
#include <iostream>
#include <vector>

std::ostream &operator<<(std::ostream &output, const std::shared_ptr<CloudSync::Resource>& resource) {
    if(resource->is_file()) {
        std::string revision = std::dynamic_pointer_cast<CloudSync::File>(resource)->revision();
        output << std::setw(10) << std::left << (revision.size() > 10 ? revision.substr(0, 10) : revision) << " " << resource->name();
    } else {
        output << std::setw(10) << std::left << "d" << " " << resource->name();
    }
    return output;
}

int main(int argc, char *argv[]) {
    std::string providerUrl;
    std::shared_ptr<CloudSync::Cloud> cloud;
    std::shared_ptr<CloudSync::Directory> dir;
    std::string root = "root";

    cxxopts::Options options("cloudsync", "A small commandline utility to access any cloud with libCloudSync");
    options.add_options()
            ("h,help", "Print help");
    options.add_options("1) cloud providers")
            ("webdav", "access webdav server")
            ("nextcloud", "access nextcloud server")
            ("owncloud", "access owncloud server")
            ("dropbox", "access dropbox cloud")
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
    } else if(result.count("webdav") || result.count("nextcloud")) {
        std::cerr << "no domain provided" << std::endl;
        std::cout << options.help({"2) configuration"}) << std::endl;
        exit(1);
    }

    if (result.count("root")) {
        root = result["root"].as<std::string>();
    }

    if(result.count("username") && result.count("password")) {
        const auto username = result["username"].as<std::string>();
        const auto password = result["password"].as<std::string>();
        auto credentials = CloudSync::BasicCredentials::from_username_password(username, password);
        if (result.count("webdav")) {
            cloud = CloudSync::CloudFactory().create_webdav(providerUrl, credentials);
        } else if (result.count("nextcloud")) {
            cloud = CloudSync::CloudFactory().create_nextcloud(providerUrl, credentials);
        } else {
            std::cerr << "no cloud provider of list [webdav, nextcloud] specified" << std::endl;
            std::cout << options.help({"1) cloud providers"}) << std::endl;
            exit(1);
        }
    } else if(result.count("token")) {
        auto credentials = CloudSync::OAuth2Credentials::from_access_token(result["token"].as<std::string>());
        if (result.count("dropbox")) {
            cloud = CloudSync::CloudFactory().create_dropbox(credentials);
        } else if (result.count("onedrive")) {
            cloud = CloudSync::CloudFactory().create_onedrive(credentials, root);
        } else if (result.count("gdrive")) {
            cloud = CloudSync::CloudFactory().create_gdrive(credentials, root);
        } else {
            std::cerr << "no cloud provider of list [dropbox, onedrive, gdrive] specified" << std::endl;
            std::cout << options.help({"1) cloud providers"}) << std::endl;
            exit(1);
        }
    } else {
        std::cerr << "no username/password or token provided" << std::endl;
        std::cout << options.help({"3) authentication"}) << std::endl;
        exit(1);
    }

    std::cout << "[1A" << std::setfill(' ') << std::left << std::setw(80) << "[Cloud url: " << cloud->get_base_url() << "]" << std::endl;
    std::cout << "Logged in as: " << cloud->get_user_display_name() << std::endl;
    dir = cloud->root();

    // cli loop waiting for commands
    while (true) {
        try {
            std::cout << dir->path() << "> ";
            std::string action;
            std::cin >> action;

            if (action == "ls") {
                auto list = dir->list_resources();
                std::cout << "total: " << list.size() << std::endl;
                for (const auto& res : list) {
                    std::cout << res << std::endl;
                }
            } else if (action == "cd") {
                std::string path;
                std::cin >> path;
                dir = dir->get_directory(path);
            } else if (action == "pwd") {
                std::cout << dir->path() << std::endl;
            } else if (action == "file") {
                std::string filename;
                std::cin >> filename;
                auto file = dir->get_file(filename);
                std::cout << file << std::endl;
            } else if (action == "read") {
                std::string filename;
                std::cin >> filename;
                auto file = dir->get_file(filename);
                std::cout << file->read_as_string() << std::endl;
            } else if (action == "mkdir") {
                std::string dirname;
                std::cin >> dirname;
                dir = dir->create_directory(dirname);
            } else if (action == "rmdir") {
                std::string dirname;
                std::cin >> dirname;
                auto rmdir = dir->get_directory(dirname);
                std::cout << "Are you sure you want to delete the folder '" << rmdir->path() << "'? (y/n) ";
                std::string confirmation;
                std::cin >> confirmation;
                if (confirmation == "y") {
                    rmdir->remove();
                }
            } else if (action == "touch") {
                std::string filename;
                std::cin >> filename;
                dir->create_file(filename);
            } else if (action == "rm") {
                std::string filename;
                std::cin >> filename;
                auto file = dir->get_file(filename);
                std::cout << "Are you sure you want to delete the file '" << file->path() << "'? (y/n) ";
                std::string confirmation;
                std::cin >> confirmation;
                if (confirmation == "y") {
                    file->remove();
                }
            } else if (action == "write") {
                std::string filename;
                std::cin >> filename;
                auto file = dir->get_file(filename);
                std::cout << "Write file content in quotation marks.\n"
                             "This will override the current file content!!!!!"
                          << std::endl;
                std::string inputline;
                std::string filecontent;
                while (std::cin >> inputline) {
                    filecontent += inputline + "\n";
                    if(filecontent[0] == '"') {
                        if(filecontent.size() > 1 && filecontent[filecontent.size() - 2] == '"') {
                            filecontent = filecontent.substr(1, filecontent.size() - 3);
                            filecontent += "\n";
                            break;
                        }
                    } else {
                        break;
                    }
                }
                file->write_string(filecontent);
            } else if (action == "logout") {
                std::cout << "Logging out will revoke your access-token / app password, if possible.\n"
                             "Do you want to proceed? (y/n)" << std::endl;
                std::string confirmation;
                std::cin >> confirmation;
                if (confirmation == "y") {
                    cloud->logout();
                }
            } else if (action == "exit") {
                exit(0);
            } else {
                std::cout << "unknown command. available commands: ls, cd <dir>, pwd, file <filename>, read <filename>, write <filename>, mkdir <dirname>, rmdir <dirname>, touch <filename>, rm <filename>, exit" << std::endl;
            }
        } catch (CloudSync::exceptions::Exception& e) {
            std::cout << "ERROR: " << e.what() << std::endl;
        }
    }
    return 0;
}