# Set minimum required CMake version.
cmake_minimum_required(VERSION 3.16)
# Set project name.
project(Glad)
if (WIN32)
    add_library(glad
        ../glad.c
        ../../include/glad/glad.h
    )
    target_include_directories(glad PRIVATE ../../include)
endif()