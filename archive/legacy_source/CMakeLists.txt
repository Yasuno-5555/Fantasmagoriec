cmake_minimum_required(VERSION 3.20)
project(Fantasmagoriec VERSION 3.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(FetchContent)

# === GLFW ===
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

# === STB ===
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

# === FreeType ===
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

# === Include directories ===
include_directories(src)
include_directories(${stb_SOURCE_DIR})
include_directories(${freetype_SOURCE_DIR}/include)

# === GLAD (v3 bundled) ===
set(GLAD_SOURCES
    src/platform/glad/glad.c
)

# === Core sources ===
set(CORE_SOURCES
    src/core/context.cpp
    src/core/layout.cpp
    src/core/debug.cpp
)

# === Render sources (v3 backend) ===
set(RENDER_SOURCES
    src/render/shader.cpp
    src/platform/opengl.cpp
)

# === Widget sources ===
set(WIDGET_SOURCES
    src/fanta_unified_impl.cpp
)

# === Widget Demo Executable ===
add_executable(WidgetDemo
    src/demos/widget_demo.cpp
    ${GLAD_SOURCES}
    ${CORE_SOURCES}
    ${RENDER_SOURCES}
    ${WIDGET_SOURCES}
)

target_include_directories(WidgetDemo PRIVATE
    src
    src/platform/glad
    ${glfw_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
    ${freetype_SOURCE_DIR}/include
)

target_link_libraries(WidgetDemo PRIVATE
    glfw
    freetype
    opengl32
)

# === V3 Demo (original) ===
add_executable(FantaDemo
    src/demos/demo.cpp
    ${GLAD_SOURCES}
    ${RENDER_SOURCES}
)

target_include_directories(FantaDemo PRIVATE
    src
    src/platform/glad
    ${glfw_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
    ${freetype_SOURCE_DIR}/include
)

target_link_libraries(FantaDemo PRIVATE
    glfw
    freetype
    opengl32
)
