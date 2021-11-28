from conans import ConanFile, CMake, tools, errors


def get_version():
    version = ""
    try:
        version = tools.Git().run("describe --tags")
        tools.save("VERSION", version)
    except:
        version = tools.load("VERSION")
    return version[1:]


class LibCloudSyncConan(ConanFile):
    name = "libcloudsync"
    url = "https://github.com/jothepro/libCloudSync"
    version = get_version()
    description = """A simple to use C++ interface to interact with cloud storage providers."""
    settings = "os", "compiler", "build_type", "arch"
    license = "AGPL-3.0-or-later"
    generators = "cmake_find_package", "cmake_paths"
    exports = "VERSION"
    exports_sources = "lib/*", "test/*", "cmake/*", "example/*", "it/*", "VERSION", "LICENSE", "CMakeLists.txt"
    author = "jothepro"
    options = {
        "shared": [True, False],
        "fPIC": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "fakeit:integration": "catch",
        "libcurl:with_ftp": False,
        "libcurl:with_imap": False,
        "libcurl:with_mqtt": False,
        "libcurl:with_pop3": False,
        "libcurl:with_rtsp": False,
        "libcurl:with_smb": False,
        "libcurl:with_smtp": False,
        "libcurl:with_tftp": False,
    }
    requires = (
        "nlohmann_json/3.9.1",
        "pugixml/1.11",
        "libcurl/7.79.1",
        ("cxxopts/2.2.1", "private"),
        ("catch2/2.13.4", "private"),
        ("fakeit/2.0.7", "private")
    )

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def build(self):
        cmake = CMake(self)
        if not tools.get_env("CONAN_RUN_TESTS", True):
            cmake.definitions["BUILD_TESTING"] = "OFF"
        cmake.configure()
        cmake.build()
        if tools.get_env("CONAN_RUN_TESTS", True):
            cmake.test()
        cmake.install()

    def package_info(self):
        self.cpp_info.names["cmake_find_package"] = "CloudSync"
        self.cpp_info.names["cmake_find_package_multi"] = "CloudSync"
        self.cpp_info.libs = ["CloudSync"]
