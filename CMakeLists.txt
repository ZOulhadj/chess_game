cmake_minimum_required(VERSION 3.21)
project(chess C)

set(CMAKE_C_STANDARD 23)
add_definitions(-DGL_SILENCE_DEPRECATION)

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_subdirectory(dependencies/cglm)

add_executable(chess main.c)

target_link_libraries(chess OpenGL::GL glfw GLEW::glew cglm)
