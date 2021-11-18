from conans import ConanFile, CMake, tools
import os


class CloudSyncTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package", "cmake_paths"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dylib*", dst="bin", src="lib")

    def test(self):
        if not tools.cross_building(self):
            os.chdir("bin")
            self.run(".%sPackageTest" % os.sep)
