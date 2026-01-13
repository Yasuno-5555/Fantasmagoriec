# Fantasmagorie v2 - Playground Module
cmake_minimum_required(VERSION 3.16)

add_library(fanta_playground STATIC
    playground.cpp
)

target_include_directories(fanta_playground PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(fanta_playground PUBLIC fanta_widgets fanta_core fanta_render)

target_compile_features(fanta_playground PUBLIC cxx_std_17)

add_library(fanta::playground ALIAS fanta_playground)
