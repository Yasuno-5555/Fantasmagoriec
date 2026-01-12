# Fantasmagorie v2 - Core Module
cmake_minimum_required(VERSION 3.16)

# Core library (Header-only + compiled sources)
add_library(fanta_core STATIC
    context.cpp
    layout.cpp
)

target_include_directories(fanta_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_features(fanta_core PUBLIC cxx_std_17)

# Alias for cleaner imports
add_library(fanta::core ALIAS fanta_core)
