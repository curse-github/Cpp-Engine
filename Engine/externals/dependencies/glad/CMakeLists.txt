# Set minimum required CMake version.
cmake_minimum_required(VERSION 3.16)
# Set project name.
project(Glad)
if (WIN32)
    add_library(Glad
        ../glad.c
    )
    target_include_directories(Glad PRIVATE ../../include)
endif()