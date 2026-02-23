cmake_minimum_required(VERSION 3.16)
project(MovieTheaterBooking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif()

option(BUILD_TESTS "Build unit tests" ON)
option(ENABLE_PACKAGE_CONSUMER_TEST "Enable CTest integration test for installed package consumption" OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(PROJECT_VERSION "1.0.0")
set(MOVIE_BOOKING_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/MovieTheaterBooking")

file(GLOB_RECURSE MOVIE_BOOKING_FORMAT_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/tests/*.cpp"
    "${CMAKE_SOURCE_DIR}/consumer-example/*.cpp"
)

file(GLOB_RECURSE MOVIE_BOOKING_MARKDOWN_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/*.md"
)
list(FILTER MOVIE_BOOKING_MARKDOWN_FILES EXCLUDE REGEX "/build/")
list(FILTER MOVIE_BOOKING_MARKDOWN_FILES EXCLUDE REGEX "/dist/")
list(FILTER MOVIE_BOOKING_MARKDOWN_FILES EXCLUDE REGEX "/\.git/")

find_program(CLANG_FORMAT_BIN NAMES clang-format)
if(CLANG_FORMAT_BIN)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_BIN} -i ${MOVIE_BOOKING_FORMAT_FILES}
        COMMENT "Formatting C++ sources with clang-format"
        VERBATIM
    )
else()
    add_custom_target(format
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found. Install it to use the format target."
        VERBATIM
    )
endif()

find_program(MDFORMAT_BIN NAMES mdformat)
find_program(PRETTIER_BIN NAMES prettier)

if(MDFORMAT_BIN)
    add_custom_target(format-md
        COMMAND ${MDFORMAT_BIN} ${MOVIE_BOOKING_MARKDOWN_FILES}
        COMMENT "Formatting Markdown files with mdformat"
        VERBATIM
    )
elseif(PRETTIER_BIN)
    add_custom_target(format-md
        COMMAND ${PRETTIER_BIN} --write ${MOVIE_BOOKING_MARKDOWN_FILES}
        COMMENT "Formatting Markdown files with prettier"
        VERBATIM
    )
else()
    add_custom_target(format-md
        COMMAND ${CMAKE_COMMAND} -E echo "No Markdown formatter found. Install 'mdformat' or 'prettier' to use format-md."
        VERBATIM
    )
endif()

add_custom_target(format-all
    DEPENDS format format-md
)

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_CONFIG_IN ${CMAKE_SOURCE_DIR}/cmake/Doxyfile.in)
    set(DOXYGEN_CONFIG_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_CONFIG_IN} ${DOXYGEN_CONFIG_OUT} @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
else()
    add_custom_target(docs
        COMMAND ${CMAKE_COMMAND} -E echo "Doxygen not found. Install Doxygen to use the docs target."
        VERBATIM
    )
endif()

find_program(CONAN_BIN NAMES conan)
if(CONAN_BIN)
    add_custom_target(conan-profile-detect
        COMMAND ${CONAN_BIN} profile detect --force -vwarning
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Detecting Conan profile"
        VERBATIM
    )

    add_custom_target(conan-nglog-create
        COMMAND ${CONAN_BIN} create conan/recipes/ng-log --version=0.8.2 --build=missing -s build_type=Release -vwarning
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Building local ng-log Conan package"
        VERBATIM
    )

    add_custom_target(conan-install
        COMMAND ${CONAN_BIN} install conanfile.py --output-folder=build/conan --build=missing -s build_type=Release -vwarning
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Installing Conan dependencies (warning log level)"
        VERBATIM
    )
    add_dependencies(conan-install conan-profile-detect conan-nglog-create)

    add_custom_target(conan-install-debug
        COMMAND ${CONAN_BIN} install conanfile.py --output-folder=build/conan --build=missing -s build_type=Debug -vdebug
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Installing Conan dependencies (debug log level)"
        VERBATIM
    )
    add_dependencies(conan-install-debug conan-profile-detect conan-nglog-create)

    add_custom_target(conan-install-trace
        COMMAND ${CONAN_BIN} install conanfile.py --output-folder=build/conan --build=missing -s build_type=Release -vtrace
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Installing Conan dependencies (trace log level)"
        VERBATIM
    )
    add_dependencies(conan-install-trace conan-profile-detect conan-nglog-create)
else()
    add_custom_target(conan-install
        COMMAND ${CMAKE_COMMAND} -E echo "Conan not found in PATH."
        VERBATIM
    )
    add_custom_target(conan-install-debug
        COMMAND ${CMAKE_COMMAND} -E echo "Conan not found in PATH."
        VERBATIM
    )
    add_custom_target(conan-install-trace
        COMMAND ${CMAKE_COMMAND} -E echo "Conan not found in PATH."
        VERBATIM
    )
endif()

add_library(movie_booking
    src/booking_error.cpp
    src/booking_service.cpp
    src/movie.cpp
    src/show.cpp
    src/theater.cpp
)

target_include_directories(movie_booking
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set(CONAN_GENERATOR_PATHS
    "${CMAKE_BINARY_DIR}/generators"
    "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/generators"
    "${CMAKE_BINARY_DIR}/conan/build/Release/generators"
    "${CMAKE_BINARY_DIR}/conan/build/${CMAKE_BUILD_TYPE}/generators"
    "${CMAKE_SOURCE_DIR}/build/conan/build/Release/generators"
    "${CMAKE_SOURCE_DIR}/build/conan/build/Debug/generators"
)

find_package(ng-log CONFIG REQUIRED
    PATHS ${CONAN_GENERATOR_PATHS}
    NO_DEFAULT_PATH
)

function(movie_booking_link_ng_log target_name)
    target_link_libraries(${target_name}
        PRIVATE
        ng-log::ng-log
    )
    if(WIN32)
        target_link_libraries(${target_name} PRIVATE dbghelp)
    endif()
    target_compile_definitions(${target_name} PRIVATE NGLOG_USE_EXPORT)
    if(DEFINED ng-log_INCLUDE_DIRS AND NOT "${ng-log_INCLUDE_DIRS}" STREQUAL "")
        target_include_directories(${target_name} PRIVATE ${ng-log_INCLUDE_DIRS})
    endif()
endfunction()

movie_booking_link_ng_log(movie_booking)

add_executable(movie_booking_cli
    src/main.cpp
)

target_link_libraries(movie_booking_cli
    PRIVATE
    movie_booking
)
movie_booking_link_ng_log(movie_booking_cli)

install(TARGETS movie_booking movie_booking_cli
    EXPORT MovieTheaterBookingTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT MovieTheaterBookingTargets
    FILE MovieTheaterBookingTargets.cmake
    NAMESPACE MovieTheaterBooking::
    DESTINATION ${MOVIE_BOOKING_INSTALL_CMAKEDIR}
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MovieTheaterBookingConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MovieTheaterBookingConfig.cmake
    INSTALL_DESTINATION ${MOVIE_BOOKING_INSTALL_CMAKEDIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MovieTheaterBookingConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/MovieTheaterBookingConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/MovieTheaterBookingConfigVersion.cmake
    DESTINATION ${MOVIE_BOOKING_INSTALL_CMAKEDIR}
)


if(BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED
        PATHS ${CONAN_GENERATOR_PATHS}
        NO_DEFAULT_PATH
    )
    enable_testing()

    add_executable(movie_booking_tests
        tests/src/test_booking_service.cpp
        tests/src/test_show.cpp
    )

    target_link_libraries(movie_booking_tests
        PRIVATE
        movie_booking
        GTest::gtest_main
    )
    movie_booking_link_ng_log(movie_booking_tests)
    target_include_directories(movie_booking_tests PRIVATE ${CMAKE_SOURCE_DIR}/tests/include)
    # Add GTest include dirs for CI compatibility
    if(GTest_INCLUDE_DIRS)
        target_include_directories(movie_booking_tests PRIVATE ${GTest_INCLUDE_DIRS})
    endif()

    include(GoogleTest)
    gtest_discover_tests(movie_booking_tests
        DISCOVERY_MODE PRE_TEST
        DISCOVERY_TIMEOUT 30
    )

    if(ENABLE_PACKAGE_CONSUMER_TEST)
        add_test(
            NAME package_consumer_flow
            COMMAND ${CMAKE_COMMAND}
            -DROOT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DROOT_BINARY_DIR=${CMAKE_BINARY_DIR}
            -DINSTALL_PREFIX=${CMAKE_BINARY_DIR}/package_test_install
            -DEXTRA_TOOLCHAIN_FILE=${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake
            -DTEST_CONFIG=$<CONFIG>
            -P ${CMAKE_SOURCE_DIR}/cmake/run_package_consumer_test.cmake
        )
    endif()
endif()

set(CPACK_PACKAGE_NAME "MovieTheaterBooking")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "MovieTheater")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Thread-safe in-memory movie ticket booking backend service in C++")
set(CPACK_GENERATOR "TGZ;ZIP")
include(CPack)
