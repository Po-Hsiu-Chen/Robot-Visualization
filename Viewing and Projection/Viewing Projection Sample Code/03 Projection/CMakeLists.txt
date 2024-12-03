cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

message("CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")

# Define project name and detail
project("02 Viewing")

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

# Define the executable name of this project, and create a CMake binary target
add_executable(Project)
add_executable(View1)

target_sources(Project PRIVATE "project.c")
target_sources(View1 PRIVATE "view1.c")

# Assign source files to the executable.
target_link_libraries(Project PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
target_link_libraries(View1 PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(Project PRIVATE X11)
    target_link_libraries(View1 PRIVATE X11)
endif()