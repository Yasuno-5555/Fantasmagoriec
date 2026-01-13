# Fantasmagorie v3 - CMake Build
cmake_minimum_required(VERSION 3.16)
project(Fantasmagorie_v3 VERSION 3.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GLFW (should be fetched by parent CMakeLists.txt)
# For standalone build, you'd need to fetch it

# Core library (header-only for now)
add_library(fanta_v3_core INTERFACE)
target_include_directories(fanta_v3_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Demo executable
add_executable(fanta_v3_demo
    demo.cpp
    platform/opengl.cpp
    platform/glad/glad.c
    core/shader.cpp
)

# Typography (Optional)
option(FANTA_ENABLE_TEXT "Enable high-quality typography with FreeType" ON)

if(FANTA_ENABLE_TEXT)
    # Freetype is provided by root CMakeLists.txt via FetchContent
    target_link_libraries(fanta_v3_demo PRIVATE freetype)
    target_include_directories(fanta_v3_demo PRIVATE ${freetype_SOURCE_DIR}/include)
    target_compile_definitions(fanta_v3_demo PRIVATE FANTA_ENABLE_TEXT)
endif()

if(MSVC)
    target_compile_options(fanta_v3_demo PRIVATE /utf-8)
endif()



target_link_libraries(fanta_v3_demo PRIVATE
    fanta_v3_core
    glfw
    opengl32
)

# Includes (Common)
target_include_directories(fanta_v3_demo PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/glad
)



# Copy shaders to build directory for hot-reload
add_custom_command(TARGET fanta_v3_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        ${CMAKE_BINARY_DIR}/shaders
)
