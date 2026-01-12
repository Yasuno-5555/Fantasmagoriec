# Fantasmagorie v2 - Render Module
cmake_minimum_required(VERSION 3.16)

# Render library
add_library(fanta_render STATIC
    opengl/backend_gl.cpp
)

target_include_directories(fanta_render PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(fanta_render PUBLIC fanta_core)

target_compile_features(fanta_render PUBLIC cxx_std_17)

# Alias
add_library(fanta::render ALIAS fanta_render)
