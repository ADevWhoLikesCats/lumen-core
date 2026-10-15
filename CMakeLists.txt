cmake_minimum_required(VERSION 3.16)
project(lumen-core CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------
find_package(OpenGL REQUIRED)
find_package(glfw3  REQUIRED)

# Bullet is optional. If not found, engine builds without physics.
find_package(Bullet QUIET)
if(Bullet_FOUND)
    message(STATUS "lumen: Bullet found -> physics enabled")
else()
    message(WARNING "lumen: Bullet not found -> physics subsystem disabled")
endif()

# ---------------------------------------------------------------
# Sources
# ---------------------------------------------------------------
file(GLOB_RECURSE LUMEN_SRC
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.c
)

# Don't pick up backups or editor droppings.
list(FILTER LUMEN_SRC EXCLUDE REGEX "\\.(bak|broken|orig|swp)$")

add_executable(lumen ${LUMEN_SRC})

# ---------------------------------------------------------------
# Include paths
# ---------------------------------------------------------------
target_include_directories(lumen PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/stb
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/render
    ${CMAKE_SOURCE_DIR}/src/scene
    ${CMAKE_SOURCE_DIR}/src/physics
    ${CMAKE_SOURCE_DIR}/src/audio
)

# ---------------------------------------------------------------
# Link
# ---------------------------------------------------------------
target_link_libraries(lumen PRIVATE
    OpenGL::GL
    glfw
)

if(Bullet_FOUND)
    target_compile_definitions(lumen PRIVATE LUMEN_HAVE_BULLET=1)
    target_link_libraries(lumen PRIVATE
        BulletDynamics
        BulletCollision
        LinearMath
    )
endif()

# miniaudio is header-only; we'll define the implementation once in audio.cpp.
# Harmless to define now even if no file uses it yet.
# target_compile_definitions(lumen PRIVATE MINIAUDIO_IMPLEMENTATION=1)

# ---------------------------------------------------------------
# Platform extras
# ---------------------------------------------------------------
if(APPLE)
    target_link_libraries(lumen PRIVATE
        "-framework Cocoa"
        "-framework OpenGL"
        "-framework IOKit"
        "-framework CoreVideo"
    )
elseif(WIN32)
    target_link_libraries(lumen PRIVATE opengl32)
endif()

# ---------------------------------------------------------------
# Warnings — match the Makefile
# ---------------------------------------------------------------
if(MSVC)
    target_compile_options(lumen PRIVATE /W3)
else()
    target_compile_options(lumen PRIVATE -Wall -Wextra)
endif()
