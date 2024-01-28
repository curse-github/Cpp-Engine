# Set minimum required CMake version.
cmake_minimum_required(VERSION 3.16)
# Set project name.
project(gladProj)
if (WIN32)
    add_library(glad
        ../glad.c
    )
    target_include_directories(glad PRIVATE ../../include)
endif()