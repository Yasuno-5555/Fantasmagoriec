# Fantasmagorie v2 - Widgets Module
cmake_minimum_required(VERSION 3.16)

add_library(fanta_widgets STATIC
    button.cpp
    label.cpp
    slider.cpp
    checkbox.cpp
)

target_include_directories(fanta_widgets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(fanta_widgets PUBLIC fanta_core fanta_render)

target_compile_features(fanta_widgets PUBLIC cxx_std_17)

add_library(fanta::widgets ALIAS fanta_widgets)
