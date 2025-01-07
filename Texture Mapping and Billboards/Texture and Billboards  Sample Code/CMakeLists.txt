cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

message("CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")

# Define project name and detail
project("05 Texture")

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

# Define the executable name of this project, and create a CMake binary target
add_executable(AliasText)
add_executable(Billborad)
add_executable(Blend)
add_executable(CopyTexture)
add_executable(Mipmap)
add_executable(Texture)
add_executable(TextureMap)

target_sources(AliasText PRIVATE "alias_text.c")
target_sources(Billborad PRIVATE "billboard.c")
target_sources(Blend PRIVATE "blend.c")
target_sources(CopyTexture PRIVATE "copy_texture.c")
target_sources(Mipmap PRIVATE "mipmap.c")
target_sources(Texture PRIVATE "texture.c")
target_sources(TextureMap PRIVATE "TextureMap.cpp")


# Assign source files to the executable.
target_link_libraries(AliasText PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
target_link_libraries(Billborad PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
target_link_libraries(Blend PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
target_link_libraries(CopyTexture PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
target_link_libraries(Mipmap PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
target_link_libraries(Texture PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
target_link_libraries(TextureMap PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(AliasText PRIVATE X11)
    target_link_libraries(Billborad PRIVATE X11)
    target_link_libraries(Blend PRIVATE X11)
    target_link_libraries(CopyTexture PRIVATE X11)
    target_link_libraries(Mipmap PRIVATE X11)
    target_link_libraries(Texture PRIVATE X11)
    target_link_libraries(TextureMap PRIVATE X11)
endif()