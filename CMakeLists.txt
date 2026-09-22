cmake_minimum_required(VERSION 3.16)
project(aether_broker CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Cờ cảnh báo biên dịch nghiêm ngặt
add_compile_options(-Wall -Wextra -Wpedantic)

# --- 1. CẤU HÌNH SANITIZER (Công tắc soi lỗi bộ nhớ / race condition) ---
set(AETHER_SANITIZE "" CACHE STRING "Options: address | thread | undefined")
if(AETHER_SANITIZE)
    message(STATUS "Enabling Sanitizer: ${AETHER_SANITIZE}")
    add_compile_options(-fsanitize=${AETHER_SANITIZE} -fno-omit-frame-pointer)
    add_link_options(-fsanitize=${AETHER_SANITIZE})
endif()

# Thư mục chứa file header (.hpp)
include_directories(include)

# --- 2. FILE CHẠY CHÍNH ---
add_executable(aether_broker src/main.cpp)

find_package(Threads REQUIRED)
target_link_libraries(aether_broker PRIVATE Threads::Threads)

# --- 3. TÍCH HỢP GOOGLETEST VIA FETCHCONTENT ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
# Bỏ qua cài đặt các bài test mặc định của GTest để build nhanh hơn
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Đánh dấu bật tính năng testing cho CMake
enable_testing()

# Tạo target test riêng
add_executable(aether_tests test/test_queue.cpp)
target_link_libraries(aether_tests PRIVATE GTest::gtest_main Threads::Threads)

# Tự động nhận diện bài test cho ctest
include(GoogleTest)
gtest_discover_tests(aether_tests)