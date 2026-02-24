from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import get
import os


class NgLogConan(ConanFile):
    name = "ng-log"
    version = "0.8.2"
    package_type = "library"

    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    exports_sources = "*"

    def _append_install_rules_if_missing(self):
        cmakelists = os.path.join(self.source_folder, "CMakeLists.txt")
        if not os.path.exists(cmakelists):
            return

        with open(cmakelists, "r+", encoding="utf-8") as file:
            content = file.read()
            patch = ""

            if "install(TARGETS" not in content:
                patch += (
                    "\ninstall(TARGETS ng-log EXPORT ng-logTargets "
                    "ARCHIVE DESTINATION lib LIBRARY DESTINATION lib "
                    "RUNTIME DESTINATION bin INCLUDES DESTINATION include)\n"
                )

            include_dir = os.path.join(self.source_folder, "include")
            if os.path.isdir(include_dir) and "install(DIRECTORY include/ DESTINATION include)" not in content:
                patch += (
                    "install(DIRECTORY include/ DESTINATION include FILES_MATCHING "
                    "PATTERN \"*.h\" PATTERN \"*.hpp\")\n"
                )

            if patch:
                file.seek(0, os.SEEK_END)
                file.write(patch)

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
        self._append_install_rules_if_missing()
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "ng-log")
        self.cpp_info.set_property("cmake_target_name", "ng-log::ng-log")
        # Use ng-logd for Debug, ng-log for other build types
        if self.settings.build_type == "Debug":
            self.cpp_info.libs = ["ng-logd"]
        else:
            self.cpp_info.libs = ["ng-log"]
        self.cpp_info.defines.append("NGLOG_USE_EXPORT")
        if not self.options.shared:
            self.cpp_info.defines.append("NGLOG_STATIC_DEFINE")