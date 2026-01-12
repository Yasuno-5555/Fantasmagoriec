cmake_minimum_required(VERSION 3.20)
project(Fantasmagoriec VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)


# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLAD
# We will generate glad during build or include it. 
# For simplicity, let's use a pre-generated glad or a fetch content wrapper if available.
# Actually, the most robust way for this environment without pre-installed glad is to download the files or use a repo.
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.4
)
# GLAD cmake support is sometimes tricky, let's try to just fetch it and include headers/sources manually if standard install fails,
# but Dav1dde/glad usually works if we generate.
# However, for a quick setup, using a simplified loader like `gl3w` or manually adding glad.{c,h} is safer.
# Let's try to grab a simple single-header GL loader repo or just assume user has system GLFW/GLAD?
# The user said "Dependencies: Minimal (GLFW, GLAD/OpenGL, STB)".
# I'll stick to GLFW via FetchContent. For GLAD, I might need to generate it or include it.
# Let's assume we can fetch a ready-to-use CMake-ified glad or similar.
# A popular alternative is just using the files. I will attempt to download a cmake-ready glad.
FetchContent_Declare(
    glad-cmake
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.4
)
# Wait, generating glad requires python/execution during config.
# To be safe and "Offline-ish" friendly if possible, but FetchContent needs net.
# I will use a simple approach: Just fetch GLFW. I'll add a dummy GL loader or assume system GLAD for now.
# Actually, let's look for a header-only or simple GL loader that CMake can easier handle.
# "gl3w" or similar. Or, I will just put the glad files in /thirdparty if I could.
# Plan B: Just using GLFW's native context and plain OS headers? No, we need extensions.
# I will try to use `glbinding` or just standard `glad` assuming it works.
# Let's assume the user can provide GLAD or I can add a placeholder.
# I will create a `third_party` folder and putting a placeholder script to populate it,
# BUT for now I will try to use FetchContent for GLAD as well, hoping for the best.

# STB
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

# --- FreeType ---
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(freetype)




# Includes
include_directories(src)

# Source files
set(SOURCES 
    src/main.cpp
    src/core/node.hpp
    src/core/ui_context.hpp
    src/core/ui_context.cpp
    src/core/application.hpp
    src/core/application.cpp
    src/core/text_buffer.hpp
    src/layout/flex_layout.hpp
    src/layout/flex_layout.cpp
    src/render/renderer.hpp
    src/render/renderer.cpp
    src/render/font_manager.hpp
    src/render/font_manager.cpp
    src/platform/gl_lite.hpp
    src/platform/gl_lite.cpp
    src/platform/ime_win32.hpp
    src/platform/ime_win32.cpp
    src/input/focus.hpp
    src/input/focus.cpp
    src/input/input_dispatch.hpp
    src/input/input_dispatch.cpp
    src/ui/api.cpp
)















# Executable
add_executable(Fantasmagorie ${SOURCES})

# Test STB
add_executable(TestSTB test_stb.cpp)
target_include_directories(TestSTB PRIVATE ${stb_SOURCE_DIR})


target_link_libraries(Fantasmagorie PRIVATE glfw)
if(WIN32)
    target_link_libraries(Fantasmagorie PRIVATE imm32 user32 gdi32 shell32 opengl32 freetype)
endif()

# Test Compile
add_executable(TestCompile src/test_compile.cpp)
target_include_directories(TestCompile PRIVATE 
    ${glfw_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
    ${freetype_SOURCE_DIR}/include
    src
)






target_include_directories(Fantasmagorie PRIVATE 
    ${glfw_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
    ${freetype_SOURCE_DIR}/include
    src
)


