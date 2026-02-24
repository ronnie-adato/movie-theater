from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout


class MovieTheaterConan(ConanFile):
    name = "movie_theater_booking"
    version = "1.0.0"
    package_type = "application"

    settings = "os", "compiler", "build_type", "arch"
    options = {"build_tests": [True, False]}
    default_options = {"build_tests": True}

    exports_sources = "CMakeLists.txt", "include/*", "src/*", "tests/*"

    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("ng-log/0.8.2")
        if self.options.build_tests:
            self.requires("gtest/1.14.0")

    def package_info(self):
        self.cpp_info.bindirs = ["bin"]

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure(variables={"BUILD_TESTS": self.options.build_tests})
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()