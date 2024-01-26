# Set minimum required CMake version.
cmake_minimum_required(VERSION 3.16)
# Set project name.
project(Glad)
if (WIN32)
    add_library(GladLib
        ../glad.c
    )
    target_include_directories(GladLib PRIVATE ../../include)
endif()