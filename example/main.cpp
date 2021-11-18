#include "CloudSync/CloudFactory.hpp"
#include "CloudSync/Directory.hpp"
#include "CloudSync/File.hpp"
#include "CloudSync/OAuth2Credentials.hpp"
#include "CloudSync/UsernamePasswordCredentials.hpp"
#include <cxxopts.hpp>
#include <iomanip>
#include <iostream>
#include <vector>

std::ostream &operator<<(std::ostream &output, const std::shared_ptr<CloudSync::Cloud>& cloud) {
    output << "[Cloud url: " << cloud->getBaseUrl() << "]" << std::endl;
    return output;
}

std::ostream &operator<<(std::ostream &output, const std::shared_ptr<CloudSync::Resource>& resource) {
    if(resource->isFile()) {
        std::string revision = std::dynamic_pointer_cast<CloudSync::File>(resource)->revision();
        output << std::setw(10) << std::left << (revision.size() > 10 ? revision.substr(0, 10) : revision) << " " << resource->name;
    } else {
        output << std::setw(10) << std::left << "d" << " " << resource->name;
    }
    return output;
}

int main(int argc, char *argv[]) {
    std::string providerUrl;
    std::shared_ptr<CloudSync::Cloud> cloud;
    std::shared_ptr<CloudSync::Directory> dir;
    std::string root;

    cxxopts::Options options("cloudsync", "A small commandline utility to access any cloud with libCloudSync");
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
            ("u,username", "username, if login with username & password should be used. You will be prompted to provide the related password.", cxxopts::value<std::string>())
            ("t,token", "OAuth2 access token. If not set, you will be prompted to provide it.", cxxopts::value<std::string>());
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
        if(result.count("webdav") || result.count("nextcloud") || result.count("owncloud")) {
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
    } else if (result.count("owncloud")) {
        // TODO
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
    if (result.count("username")) {
        while (true) {
            try {
                std::string password;
                std::cout << "password: ";
                std::cin >> password;
                auto credentials =
                        CloudSync::UsernamePasswordCredentials(result["username"].as<std::string>(), password);
                cloud->login(credentials);
                break;
            } catch (CloudSync::BaseException& e) {
                std::cerr << "\e[1A" << e.what() << std::endl;
            }
        }

    } else {
        if (result.count("token")) {
            auto credentials = CloudSync::OAuth2Credentials(result["token"].as<std::string>());
            cloud->login(credentials);
        } else {
            while (true) {
                try {
                    std::string token;
                    std::cout << "token: ";
                    std::cin >> token;
                    auto credentials = CloudSync::OAuth2Credentials(token);
                    cloud->login(credentials);
                    break;
                } catch (CloudSync::BaseException& e) {
                    std::cerr << "\e[1A" << e.what() << std::endl;
                }
            }
        }
    }

    std::cout << "\e[1A" << std::setfill(' ') << std::left << std::setw(80) << cloud << std::endl;
    std::cout << "Logged in as: " << cloud->getUserDisplayName() << std::endl;
    dir = cloud->root();

    std::cout << dir->pwd() << std::endl;

    // cli loop waiting for commands
    while (true) {
        try {
            std::cout << dir->path << "> ";
            std::string action;
            std::cin >> action;

            if (action == "ls") {
                auto list = dir->ls();
                std::cout << "total: " << list.size() << std::endl;
                for (const auto& res : list) {
                    std::cout << res << std::endl;
                }
            } else if (action == "cd") {
                std::string path;
                std::cin >> path;
                dir = dir->cd(path);
            } else if (action == "pwd") {
                std::cout << dir->pwd() << std::endl;
            } else if (action == "file") {
                std::string filename;
                std::cin >> filename;
                auto file = dir->file(filename);
                std::cout << file << std::endl;
            } else if (action == "read") {
                std::string filename;
                std::cin >> filename;
                auto file = dir->file(filename);
                std::cout << file->read() << std::endl;
            } else if (action == "mkdir") {
                std::string dirname;
                std::cin >> dirname;
                dir->mkdir(dirname);
            } else if (action == "rmdir") {
                std::string dirname;
                std::cin >> dirname;
                auto rmdir = dir->cd(dirname);
                std::cout << "Are you sure you want to delete the folder '" << rmdir->pwd() << "'? (y/n) ";
                std::string confirmation;
                std::cin >> confirmation;
                if (confirmation == "y") {
                    rmdir->rmdir();
                }
            } else if (action == "touch") {
                std::string filename;
                std::cin >> filename;
                dir->touch(filename);
            } else if (action == "rm") {
                std::string filename;
                std::cin >> filename;
                auto file = dir->file(filename);
                std::cout << "Are you sure you want to delete the file '" << file->path << "'? (y/n) ";
                std::string confirmation;
                std::cin >> confirmation;
                if (confirmation == "y") {
                    file->rm();
                }
            } else if (action == "write") {
                std::string filename;
                std::cin >> filename;
                auto file = dir->file(filename);
                std::cout << "Write file content. To input multiline text, add \" at the beginning and end of the input.\n"
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
                file->write(filecontent);
            } else if (action == "exit") {
                exit(0);
            } else {
                std::cout << "unknown command. available commands: ls, cd <dir>, pwd, file <filename>, read <filename>, write <filename>, mkdir <dirname>, rmdir <dirname>, touch <filename>, rm <filename>, exit" << std::endl;
            }
        } catch (CloudSync::BaseException& e) {
            std::cout << "ERROR: " << e.what() << std::endl;
        }
    }
    return 0;
}