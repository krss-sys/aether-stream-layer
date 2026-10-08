cmake_minimum_required(VERSION 3.16)
project(aether_broker CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Strict compiler warning flags
add_compile_options(-Wall -Wextra -Wpedantic)

# --- 1. SANITIZER CONFIGURATION ---
set(AETHER_SANITIZE "" CACHE STRING "Options: address | thread | undefined")
if(AETHER_SANITIZE)
    message(STATUS "Enabling Sanitizer: ${AETHER_SANITIZE}")
    add_compile_options(-fsanitize=${AETHER_SANITIZE} -fno-omit-frame-pointer)
    add_link_options(-fsanitize=${AETHER_SANITIZE})
endif()

# Header files directory (.hpp)
include_directories(include)

# --- 2. EXTRACT AETHER_CORE STATIC LIBRARY ---
# Automatically collect all source files in src/
file(GLOB CORE_SRC CONFIGURE_DEPENDS "src/*.cpp")
# Exclude main.cpp from the core static library
list(FILTER CORE_SRC EXCLUDE REGEX "main\\.cpp$")

# Build the aether_core static library
add_library(aether_core STATIC ${CORE_SRC})

# --- 3. MAIN SERVER EXECUTABLE ---
add_executable(aether_server src/main.cpp)

find_package(Threads REQUIRED)
# Link aether_core and Threads to the server executable
target_link_libraries(aether_server PRIVATE aether_core Threads::Threads)

# --- 4. GOOGLETEST INTEGRATION ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing functionality
enable_testing()

# Create dedicated test target
add_executable(aether_tests 
    tests/test_queue.cpp 
    tests/test_fd.cpp
)
# Link aether_core to the test executable as well
target_link_libraries(aether_tests PRIVATE aether_core GTest::gtest_main Threads::Threads)

# Create CLI executable target
add_executable(aether_cli tools/aether_cli.cpp)

# Link aether_core to the CLI executable
target_link_libraries(aether_cli PRIVATE aether_core)

# Automatically discover tests for ctest
include(GoogleTest)
gtest_discover_tests(aether_tests)