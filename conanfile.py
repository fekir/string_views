#!/usr/bin/env python

from conans import ConanFile, CMake

class chkConan(ConanFile):
    name = "string_views"
    description = "...."
    url = "https://github.com/fekir/check"
    homepage = url
    license = "BSL-1.0"
    exports = ("LICENSE", "readme")
    version = "0.1"
    settings = "os", "compiler", "arch", "build_type"
    exports_sources = ("include/*", "CMakeLists.txt", "tests/*")
    no_copy_source = True

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_id(self):
        self.info.header_only()
