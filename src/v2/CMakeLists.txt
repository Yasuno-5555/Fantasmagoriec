# Fantasmagorie v2
cmake_minimum_required(VERSION 3.16)
project(Fantasmagorie_v2 VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =============================================================================
# Core Module
# =============================================================================
add_subdirectory(core)

# =============================================================================
# Render Module
# =============================================================================
add_subdirectory(render)

# =============================================================================
# Widgets Module
# =============================================================================
add_subdirectory(widgets)
add_subdirectory(playground)

# DX Verification Target
add_library(dx_verif STATIC demos/dx_test.cpp)
target_link_libraries(dx_verif PUBLIC fanta_widgets fanta_core)
target_include_directories(dx_verif PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Demo Runner Executable (Console Mode)
add_executable(fanta_demo
    demos/demo_runner.cpp
    demos/dx_test.cpp
    platform/gl_lite.cpp
)
target_link_libraries(fanta_demo PRIVATE 
    fanta_widgets 
    fanta_core 
    fanta_render
    glfw
    opengl32
)
target_include_directories(fanta_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# =============================================================================
# Test Executable (Compile Check)
# =============================================================================
add_executable(fanta_v2_test
    test_compile.cpp
)

target_link_libraries(fanta_v2_test PRIVATE fanta::core fanta::render fanta::widgets)
