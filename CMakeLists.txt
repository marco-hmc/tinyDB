cmake_minimum_required(VERSION 3.12)
project(demo_0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(spdlog REQUIRED)

include_directories(include)

# 指定源文件
file(GLOB SOURCES "src/*.cpp")

# 添加可执行文件
add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

# 添加测试目录
enable_testing()
add_subdirectory(test)
