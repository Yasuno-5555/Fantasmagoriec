cmake_minimum_required(VERSION 3.20)

# FreeType Fix: Force policy compatibility
if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(FantasmagoriecCrystal VERSION 0.1.0 LANGUAGES CXX C)

# --- Feature Options ---
option(FANTA_BUILD_MARKDOWN    "Build Markdown widget" ON)
option(FANTA_BUILD_TIMELINE    "Build Timeline engine" ON)
option(FANTA_BUILD_GRID        "Build Grid layout" ON)
option(FANTA_BUILD_NODE_EDITOR "Build Node Editor" ON)
option(FANTA_BUILD_SCRIPTING   "Build Scripting engine" ON)
option(FANTA_BUILD_TOOLS        "Build Inspector/Profiler" ON)
option(FANTA_BUILD_VULKAN      "Build Vulkan backend" OFF)
option(FANTA_BUILD_OPENGL      "Build OpenGL backend" ON)
option(FANTA_BUILD_GLES        "Build GLES backend" ON)
option(FANTA_MOBILE            "Enable Mobile features" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# GLFW (Local)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

# STB (Remote)
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

# FreeType (Remote)
FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG        VER-2-13-2
)
FetchContent_MakeAvailable(freetype)

# HarfBuzz (Remote)
FetchContent_Declare(
    harfbuzz
    URL https://github.com/harfbuzz/harfbuzz/releases/download/8.3.0/harfbuzz-8.3.0.tar.xz
)
FetchContent_MakeAvailable(harfbuzz)

# Sources
# Base Sources
# Base Sources
# Base Sources
set(CORE_SOURCES
    src/core/context.cpp
    src/core/scheduler.cpp
    src/core/history.cpp
    src/view/view_api.cpp
    src/backend/drawlist.cpp
    src/element/layout.cpp
    src/render/render_tree.cpp
    src/text/glyph_cache.cpp
    src/text/text_layout.cpp
    src/text/font_manager.cpp
    src/text/text_shaper.cpp
    src/ui/element_builder.cpp
    src/ui/element_positioning.cpp
    src/core/theme_impl.cpp
    src/core/stb_impl.cpp
    src/glad.c
    src/ui/text_input.cpp
    src/platform/native_dialogs.cpp
    src/platform/clipboard.cpp
    src/core/undo_redo.cpp
    src/ui/gestures.cpp
    src/platform/ime.cpp
    src/platform/native_bridge.cpp
    src/platform/native_bridge_desktop.cpp
    src/core/plugin_loader.cpp
    src/core/lowering.cpp
)


if(WIN32)
    list(APPEND CORE_SOURCES src/platform/win32_dialogs.cpp)
endif()

if(FANTA_BUILD_OPENGL)
    list(APPEND CORE_SOURCES src/backend/opengl_backend.cpp)
    add_definitions(-DFANTA_BACKEND_OPENGL=1)
endif()

if(FANTA_BUILD_VULKAN)
    find_package(Vulkan REQUIRED)
    list(APPEND CORE_SOURCES src/backend/vulkan_backend.cpp)
    add_definitions(-DFANTA_BACKEND_VULKAN=1)
endif()

if(FANTA_BUILD_GLES)
    list(APPEND CORE_SOURCES src/backend/gles_backend.cpp)
    add_definitions(-DFANTA_BACKEND_GLES=1)
endif()

if(FANTA_MOBILE)
    add_definitions(-DFANTA_MOBILE=1)
endif()

if(FANTA_BUILD_TIMELINE)
    list(APPEND CORE_SOURCES src/core/timeline.cpp)
    add_definitions(-DFANTA_FEATURE_TIMELINE=1)
endif()

if(FANTA_BUILD_GRID)
    list(APPEND CORE_SOURCES src/element/grid_layout.cpp)
    add_definitions(-DFANTA_FEATURE_GRID=1)
endif()

if(FANTA_BUILD_TOOLS)
    list(APPEND CORE_SOURCES src/tools/inspector.cpp src/tools/profiler.cpp)
    add_definitions(-DFANTA_FEATURE_TOOLS=1)
endif()

if(FANTA_BUILD_SCRIPTING)
    list(APPEND CORE_SOURCES src/scripting/script_engine.cpp)
    add_definitions(-DFANTA_FEATURE_SCRIPTING=1)
endif()

# Include Dirs
include_directories(include)
include_directories(src)
include_directories(${stb_SOURCE_DIR})
include_directories(${freetype_SOURCE_DIR}/include)

# Executable

# Executables
add_executable(scroll_demo demos/scroll_demo.cpp ${CORE_SOURCES})
add_executable(rich_demo demos/rich_demo.cpp ${CORE_SOURCES})
    add_executable(interaction_demo demos/interaction_demo.cpp ${CORE_SOURCES})
    # add_executable(wire_demo demos/wire_demo.cpp ${CORE_SOURCES})
    add_executable(v5_demo demos/v5_demo.cpp ${CORE_SOURCES})

if(WIN32)
    target_link_libraries(scroll_demo PRIVATE glfw opengl32 user32 gdi32 shell32 ole32 uuid freetype harfbuzz)
    target_link_libraries(rich_demo PRIVATE glfw opengl32 user32 gdi32 shell32 ole32 uuid freetype harfbuzz)
    target_link_libraries(interaction_demo PRIVATE glfw opengl32 user32 gdi32 shell32 ole32 uuid freetype harfbuzz)
    # target_link_libraries(wire_demo PRIVATE glfw opengl32 user32 gdi32 shell32 ole32 uuid freetype harfbuzz)
    target_link_libraries(v5_demo PRIVATE glfw opengl32 user32 gdi32 shell32 ole32 uuid freetype harfbuzz)
else()
    target_link_libraries(scroll_demo PRIVATE glfw GL freetype harfbuzz::harfbuzz pthread dl Vulkan::Vulkan)
    target_link_libraries(rich_demo PRIVATE glfw GL freetype harfbuzz::harfbuzz pthread dl Vulkan::Vulkan)
    target_link_libraries(interaction_demo PRIVATE glfw GL freetype harfbuzz::harfbuzz pthread dl Vulkan::Vulkan)
    target_link_libraries(wire_demo PRIVATE glfw GL freetype harfbuzz::harfbuzz pthread dl Vulkan::Vulkan)
endif()

# Benchmark Tool
add_executable(fanta_benchmark src/tools/benchmark_main.cpp ${CORE_SOURCES})
if(WIN32)
    target_link_libraries(fanta_benchmark PRIVATE glfw opengl32 user32 gdi32 shell32 ole32 uuid freetype harfbuzz)
else()
    target_link_libraries(fanta_benchmark PRIVATE glfw GL freetype harfbuzz::harfbuzz pthread dl Vulkan::Vulkan)
endif()
