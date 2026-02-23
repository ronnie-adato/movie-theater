from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import get


class NgLogConan(ConanFile):
    name = "ng-log"
    version = "0.8.2"
    package_type = "library"

    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    exports_sources = "*"

    def layout(self):
        cmake_layout(self)

    def source(self):
        get(
            self,
            url="https://github.com/ng-log/ng-log/archive/refs/tags/v0.8.2.tar.gz",
            strip_root=True,
        )

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_TESTING"] = False
        tc.variables["WITH_GFLAGS"] = False
        tc.variables["WITH_GTEST"] = False
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "ng-log")
        self.cpp_info.set_property("cmake_target_name", "ng-log::ng-log")
        self.cpp_info.libs = ["ng-log"]
        self.cpp_info.defines.append("NGLOG_USE_EXPORT")
        if not self.options.shared:
            self.cpp_info.defines.append("NGLOG_STATIC_DEFINE")