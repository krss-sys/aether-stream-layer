cmake_minimum_required(VERSION 3.16)
project(aether_broker CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Strict compiler warning flags
add_compile_options(-Wall -Wextra -Wpedantic)

# --- 1. SANITIZER CONFIGURATION (Memory / Race Condition detection flags) ---
set(AETHER_SANITIZE "" CACHE STRING "Options: address | thread | undefined")
if(AETHER_SANITIZE)
    message(STATUS "Enabling Sanitizer: ${AETHER_SANITIZE}")
    add_compile_options(-fsanitize=${AETHER_SANITIZE} -fno-omit-frame-pointer)
    add_link_options(-fsanitize=${AETHER_SANITIZE})
endif()

# Header files directory (.hpp)
include_directories(include)

# --- 2. MAIN EXECUTABLE ---
add_executable(aether_broker src/main.cpp)

find_package(Threads REQUIRED)
target_link_libraries(aether_broker PRIVATE Threads::Threads)

# --- 3. GOOGLETEST INTEGRATION VIA FETCHCONTENT ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
# Override default GTest installation settings to speed up build
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing functionality for CMake
enable_testing()

# Create dedicated test target
add_executable(aether_tests 
    tests/test_queue.cpp 
    tests/test_fd.cpp
)
target_link_libraries(aether_tests PRIVATE GTest::gtest_main Threads::Threads)

# Automatically discover tests for ctest
include(GoogleTest)
gtest_discover_tests(aether_tests)