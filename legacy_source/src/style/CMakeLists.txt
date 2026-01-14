# Fantasmagorie v2 - Style Module
cmake_minimum_required(VERSION 3.16)

# Header-only library
add_library(fanta_style INTERFACE)

target_include_directories(fanta_style INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_library(fanta::style ALIAS fanta_style)
