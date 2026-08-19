cmake_minimum_required(VERSION 3.15)
project(llblazer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVDEV REQUIRED IMPORTED_TARGET libevdev)

add_executable(llblaze_inputer src/main.cpp)

target_include_directories(llblaze_inputer PRIVATE 
    ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(llblaze_inputer PRIVATE PkgConfig::LIBEVDEV)

