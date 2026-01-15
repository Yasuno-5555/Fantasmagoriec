cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(FantasmagoriecCrystal VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# FreeType
FetchContent_Declare(
  freetype
  GIT_REPOSITORY https://github.com/freetype/freetype.git
  GIT_TAG VER-2-13-0
)
FetchContent_MakeAvailable(freetype)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

# STB
FetchContent_Declare(stb GIT_REPOSITORY https://github.com/nothings/stb.git GIT_TAG master)
FetchContent_MakeAvailable(stb)

# pybind11
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG v2.11.1
)
FetchContent_MakeAvailable(pybind11)

# ===== V5 Core + UI Layer =====
set(CORE_SOURCES
    # V5 Core (Sacred - DO NOT MODIFY)
    src/core/v5_core.cpp
    src/backend/drawlist.cpp
    src/backend/opengl_backend_minimal.cpp
    src/backend/opengl_backend.cpp # Full Backend
    src/backend/backend_factory.cpp
    src/core/stb_impl.cpp
    src/glad.c
    
    # UI Layer (User Land)
    src/view/api.cpp
    src/view/layout.cpp
    src/view/renderer.cpp

    # Text Engine (Ported from Legacy)
    src/text/font_manager.cpp
)

include_directories(include)
include_directories(src)
include_directories(${stb_SOURCE_DIR})
include_directories(${freetype_SOURCE_DIR}/include)
include_directories(${glfw_SOURCE_DIR}/include)

add_executable(v5_demo demos/v5_demo.cpp ${CORE_SOURCES})
add_executable(interactive_demo demos/interactive_demo.cpp ${CORE_SOURCES})
add_executable(rich_demo demos/rich_demo.cpp ${CORE_SOURCES})
add_executable(juice_demo demos/juice_demo.cpp ${CORE_SOURCES})

if(WIN32)
    set(EXTRA_LIBS glfw freetype opengl32 user32 gdi32 shell32 ole32 uuid imm32)
else()
    set(EXTRA_LIBS glfw freetype GL pthread dl)
endif()

target_link_libraries(v5_demo PRIVATE ${EXTRA_LIBS})
target_link_libraries(interactive_demo PRIVATE ${EXTRA_LIBS})
target_link_libraries(rich_demo PRIVATE ${EXTRA_LIBS})
target_link_libraries(juice_demo PRIVATE ${EXTRA_LIBS})

# Python Bindings
pybind11_add_module(fanta src/python/binding.cpp ${CORE_SOURCES})
target_link_libraries(fanta PRIVATE ${EXTRA_LIBS})
