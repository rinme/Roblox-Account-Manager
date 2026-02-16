cmake_minimum_required(VERSION 3.20)
project(RobloxAccountManager VERSION 3.7.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# nlohmann/json for JSON parsing
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

# Google Test for testing
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)

# libsodium wrapper (C library, built separately on Windows)
# We use vcpkg or system-installed libsodium
find_package(unofficial-sodium CONFIG QUIET)

FetchContent_MakeAvailable(json googletest)

# Core library
add_library(ram_core
    src/ini_file.cpp
    src/account.cpp
    src/utilities.cpp
    src/cryptography.cpp
)

target_include_directories(ram_core PUBLIC include)
target_link_libraries(ram_core PUBLIC nlohmann_json::nlohmann_json)

if(unofficial-sodium_FOUND)
    target_link_libraries(ram_core PUBLIC unofficial-sodium::sodium)
    target_compile_definitions(ram_core PUBLIC RAM_HAS_LIBSODIUM=1)
else()
    message(STATUS "libsodium not found; cryptography features will be disabled")
    target_compile_definitions(ram_core PUBLIC RAM_HAS_LIBSODIUM=0)
endif()

# Main executable (placeholder)
add_executable(roblox_account_manager src/main.cpp)
target_link_libraries(roblox_account_manager PRIVATE ram_core)

# Tests
enable_testing()
add_executable(ram_tests
    tests/test_ini_file.cpp
    tests/test_account.cpp
    tests/test_utilities.cpp
    tests/test_cryptography.cpp
)

target_link_libraries(ram_tests PRIVATE ram_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(ram_tests)
