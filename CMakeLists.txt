cmake_minimum_required(VERSION 3.16)
project(aether_broker CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic)

# --- Sanitizer ---
set(AETHER_SANITIZE "" CACHE STRING "Options: address | thread | undefined")
if(AETHER_SANITIZE)
    message(STATUS "Enabling Sanitizer: ${AETHER_SANITIZE}")
    add_compile_options(-fsanitize=${AETHER_SANITIZE} -fno-omit-frame-pointer)
    add_link_options(-fsanitize=${AETHER_SANITIZE})
endif()

find_package(Threads REQUIRED)

# --- aether_core ---
file(GLOB CORE_SRC CONFIGURE_DEPENDS "src/*.cpp")
list(FILTER CORE_SRC EXCLUDE REGEX "main\\.cpp$")

add_library(aether_core STATIC ${CORE_SRC})
target_include_directories(aether_core PUBLIC include)
target_link_libraries(aether_core PUBLIC Threads::Threads)

# --- Executables ---
add_executable(aether_server src/main.cpp)
target_link_libraries(aether_server PRIVATE aether_core)

add_executable(aether_cli tools/aether_cli.cpp)
target_link_libraries(aether_cli PRIVATE aether_core)

add_executable(aether_frame_torture tools/aether_frame_torture.cpp)
target_link_libraries(aether_frame_torture PRIVATE aether_core)

# --- Tests ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

add_executable(aether_tests
    tests/test_thread_pool.cpp
    tests/test_queue.cpp
    tests/test_fd.cpp
    tests/test_frame.cpp
    tests/test_net_io.cpp
)
target_link_libraries(aether_tests PRIVATE aether_core GTest::gtest_main)
gtest_discover_tests(aether_tests)