cmake_minimum_required(VERSION 3.16)
project(AetherStream LANGUAGES CXX)

# Bật chuẩn C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Mặc định Build Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Cảnh báo nghiêm ngặt
add_compile_options(-Wall -Wextra -Wpedantic)

# Thư viện Thread
find_package(Threads REQUIRED)

# Tạo file chạy aether_broker từ src/main.cpp
add_executable(aether_broker src/main.cpp)
target_include_directories(aether_broker PRIVATE include)
target_link_libraries(aether_broker PRIVATE Threads::Threads)