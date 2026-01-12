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

# =============================================================================
# Test Executable (Compile Check)
# =============================================================================
add_executable(fanta_v2_test
    test_compile.cpp
)

target_link_libraries(fanta_v2_test PRIVATE fanta::core fanta::render fanta::widgets)
