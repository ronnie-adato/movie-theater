if(NOT DEFINED ROOT_SOURCE_DIR)
    message(FATAL_ERROR "ROOT_SOURCE_DIR is required")
endif()

if(NOT DEFINED ROOT_BINARY_DIR)
    message(FATAL_ERROR "ROOT_BINARY_DIR is required")
endif()

if(NOT DEFINED INSTALL_PREFIX)
    message(FATAL_ERROR "INSTALL_PREFIX is required")
endif()

if(NOT DEFINED EXTRA_TOOLCHAIN_FILE)
    set(EXTRA_TOOLCHAIN_FILE "")
endif()

set(IS_MULTI_CONFIG OFF)
if(EXISTS "${ROOT_BINARY_DIR}/CMakeCache.txt")
    file(STRINGS "${ROOT_BINARY_DIR}/CMakeCache.txt" config_types_line REGEX "^CMAKE_CONFIGURATION_TYPES:STRING=")
    if(config_types_line)
        set(IS_MULTI_CONFIG ON)
    endif()
endif()

if(IS_MULTI_CONFIG)
    if(NOT DEFINED TEST_CONFIG OR TEST_CONFIG STREQUAL "")
        set(TEST_CONFIG "Release")
    endif()
elseif(NOT DEFINED TEST_CONFIG OR TEST_CONFIG STREQUAL "")
    set(TEST_CONFIG "Release")
endif()

set(CONSUMER_SOURCE_DIR "${ROOT_SOURCE_DIR}/consumer-example")
set(CONSUMER_BINARY_DIR "${ROOT_BINARY_DIR}/consumer-package-test-build")

function(run_cmd)
    execute_process(
        COMMAND ${ARGN}
        RESULT_VARIABLE cmd_result
        OUTPUT_VARIABLE cmd_stdout
        ERROR_VARIABLE cmd_stderr
    )

    if(NOT cmd_result EQUAL 0)
        message(FATAL_ERROR
            "Command failed (${cmd_result}): ${ARGN}\n"
            "stdout:\n${cmd_stdout}\n"
            "stderr:\n${cmd_stderr}")
    endif()
endfunction()

if(IS_MULTI_CONFIG)
    run_cmd(${CMAKE_COMMAND} --install "${ROOT_BINARY_DIR}" --prefix "${INSTALL_PREFIX}" --config "${TEST_CONFIG}")
else()
    run_cmd(${CMAKE_COMMAND} --install "${ROOT_BINARY_DIR}" --prefix "${INSTALL_PREFIX}")
endif()

set(CONSUMER_CONFIG_ARGS
    -S "${CONSUMER_SOURCE_DIR}"
    -B "${CONSUMER_BINARY_DIR}"
    -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX}
    -DCMAKE_BUILD_TYPE=${TEST_CONFIG}
)

if(NOT EXTRA_TOOLCHAIN_FILE STREQUAL "" AND EXISTS "${EXTRA_TOOLCHAIN_FILE}")
    list(APPEND CONSUMER_CONFIG_ARGS -DCMAKE_TOOLCHAIN_FILE=${EXTRA_TOOLCHAIN_FILE})

    get_filename_component(EXTRA_TOOLCHAIN_DIR "${EXTRA_TOOLCHAIN_FILE}" DIRECTORY)
    if(EXISTS "${EXTRA_TOOLCHAIN_DIR}/ng-log-config.cmake")
        list(APPEND CONSUMER_CONFIG_ARGS -Dng-log_DIR=${EXTRA_TOOLCHAIN_DIR})
    endif()
endif()

if(NOT CONSUMER_CONFIG_ARGS MATCHES "ng-log_DIR=")
    set(NG_LOG_DIR_CANDIDATES
        "${ROOT_BINARY_DIR}/generators"
        "${ROOT_BINARY_DIR}/conan/build/Release/generators"
        "${ROOT_SOURCE_DIR}/build/conan/build/Release/generators"
        "${ROOT_SOURCE_DIR}/build/conan/build/Debug/generators"
    )

    foreach(candidate ${NG_LOG_DIR_CANDIDATES})
        if(EXISTS "${candidate}/ng-log-config.cmake")
            list(APPEND CONSUMER_CONFIG_ARGS -Dng-log_DIR=${candidate})
            break()
        endif()
    endforeach()
endif()

run_cmd(
    ${CMAKE_COMMAND}
    ${CONSUMER_CONFIG_ARGS}
)

if(IS_MULTI_CONFIG)
    run_cmd(${CMAKE_COMMAND} --build "${CONSUMER_BINARY_DIR}" --config "${TEST_CONFIG}")
else()
    run_cmd(${CMAKE_COMMAND} --build "${CONSUMER_BINARY_DIR}")
endif()

set(CONSUMER_EXE "${CONSUMER_BINARY_DIR}/consumer_example")
if(WIN32)
    set(CONSUMER_EXE "${CONSUMER_BINARY_DIR}/consumer_example.exe")
endif()

if(IS_MULTI_CONFIG AND EXISTS "${CONSUMER_BINARY_DIR}/${TEST_CONFIG}/consumer_example")
    set(CONSUMER_EXE "${CONSUMER_BINARY_DIR}/${TEST_CONFIG}/consumer_example")
elseif(IS_MULTI_CONFIG AND EXISTS "${CONSUMER_BINARY_DIR}/${TEST_CONFIG}/consumer_example.exe")
    set(CONSUMER_EXE "${CONSUMER_BINARY_DIR}/${TEST_CONFIG}/consumer_example.exe")
endif()

if(NOT EXISTS "${CONSUMER_EXE}")
    message(FATAL_ERROR "Consumer executable not found: ${CONSUMER_EXE}")
endif()

run_cmd("${CONSUMER_EXE}")
