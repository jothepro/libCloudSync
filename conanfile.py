from conans import ConanFile, CMake, tools, errors

def get_version():
    version = ""
    try:
        version = tools.Git().run("describe --tags")
        tools.save("VERSION", version)
    except:
        version = tools.load("VERSION")
    return version[1:]


class MyLibraryConan(ConanFile):
    name = "mylibrary"
    version = get_version()
    description = """A basic C++ library project template using cmake and conan."""
    settings = "os", "compiler", "build_type", "arch"
    license = "AGPL-3.0-or-later"
    generators = "cmake_find_package", "cmake_paths"
    exports = "VERSION"
    exports_sources = "src/*", "test/*", "cmake/*", "VERSION", "LICENSE", "CMakeLists.txt"
    author = "jothepro"
    options = {
        "shared": [True, False],
        "fPIC": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True
    }
    requires = (
        "nlohmann_json/3.9.1"
    )
    build_requires = (
        "catch2/2.13.4"
    )

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
        self.cpp_info.names["cmake_find_package"] = "MyLibrary"
        self.cpp_info.names["cmake_find_package_multi"] = "MyLibrary"
        self.cpp_info.libs = ["MyLibrary"]