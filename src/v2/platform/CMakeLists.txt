# Fantasmagorie v2 - Platform Module
cmake_minimum_required(VERSION 3.16)

# Header-only library
add_library(fanta_platform INTERFACE)

target_include_directories(fanta_platform INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_library(fanta::platform ALIAS fanta_platform)
