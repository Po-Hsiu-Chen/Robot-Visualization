cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

message("CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")

# Define project name and detail
project("02 Viewing")

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

# Define the executable name of this project, and create a CMake binary target
add_executable(Light)

target_sources(Light PRIVATE "shade.c")

# Assign source files to the executable.
target_link_libraries(Light PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(Light PRIVATE X11)
endif()