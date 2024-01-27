# CMakeList.txt : CMake project for Engine, include source and define
set(EXTERNALS ${CMAKE_SOURCE_DIR}/Engine/externals)
set(PROJECT_NAME EngineProj)
set(LIB_NAME Engine)

function(import_library)
    link_libraries(
        ${ARGV0}
    )
    install(FILES ${ARGV0} DESTINATION lib)
endfunction()
function(include_directory)
    include_directories(${ARGV0})
    install(DIRECTORY ${ARGV0} DESTINATION include)
endfunction()

function(LoadDefault)# Loads default includes and libraries
    include_directory(${EXTERNALS}/include)
    include_directory(Headers)
    # Link libraries.
    cmake_policy(SET CMP0072 NEW)
    find_package(OpenGL REQUIRED)
    add_subdirectory(externals/dependencies/glad/ Glad)
    link_libraries(
        OpenGL::GL
        Glad
    )
    install(FILES OpenGL::GL DESTINATION lib)
    install(FILES Glad DESTINATION lib)
endfunction()
function(LoadArchitectureSpecific)# Loads includes and libraries specific to 64 bit or 32 bit
    include_directory(${EXTERNALS}/include${ARGV0})
    import_library(${EXTERNALS}/dependencies${ARGV0}/freetyped.lib)
    import_library(${EXTERNALS}/dependencies${ARGV0}/glfw-3.3.9.bin.WIN/lib-vc2022/glfw3.lib)
    import_library(${EXTERNALS}/dependencies${ARGV0}/glfw-3.3.9.bin.WIN/lib-vc2022/glfw3dll.lib)
endfunction()
function(installSelf)
    install(TARGETS ${LIB_NAME}
        LIBRARY DESTINATION lib
    )
endfunction()

cmake_minimum_required(VERSION 3.16)# Set minimum required CMake version.
set(CMAKE_INSTALL_MESSAGE NEVER)# Make cMake be quiet
# Set visual studio to only have release and debug configurations
set(CMAKE_CONFIGURATION_TYPES Release Debug)
# Add project.
project(${PROJECT_NAME}
    VERSION 0.5
    LANGUAGES CXX
)
# Add header files.
set(HEADERS
    Headers/AudioManager.h
    Headers/BatchedRenderers.h
    Headers/Cameras.h
    Headers/Engine.h
    Headers/EngineLib.h
    Headers/Renderers.h
    Headers/UI.h
)
# Add source to this project's executable.
set(SOURCES
    AudioManager.cpp
    BatchedRenderers.cpp
    Cameras.cpp
    Engine.cpp
    EngineLib.cpp
    Renderers.cpp
    UI.cpp
)
# Add executable.
add_library(${LIB_NAME} ${HEADERS} ${SOURCES})
MESSAGE("Detecting OS.")
if (WIN32)
    MESSAGE("OS detected: Windows")
    MESSAGE("Detecting Architecture.")
    if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")# is 64 bit windows
        MESSAGE("Architecture detected: 64 bit")
        LoadDefault()
        LoadArchitectureSpecific(64)
            installSelf()
    elseif ("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "WIN32" OR "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win32")# is 32 bit windows
        MESSAGE("Architecture detected: 32 bit")
        LoadDefault()
        LoadArchitectureSpecific(32)
            installSelf()
    else()# Architecture could easily determined, try to detect a different way
        set(_x64processors AMD64 IA64 ARM64 EM64T)
        list (FIND _x64processors "${CMAKE_SYSTEM_PROCESSOR}" _index)
        if (${_index} GREATER -1)# is 64 bit windows
            MESSAGE("Architecture detected: 64 bit")
            LoadDefault()
            LoadArchitectureSpecific(64)
            installSelf()
        elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86")# is 32 bit windows
            MESSAGE("Architecture detected: 32 bit")
            LoadDefault()
            LoadArchitectureSpecific(32)
            installSelf()
        else()
            MESSAGE("Architecture could not be determined, build failed")
        endif()
    endif()
endif()

if (CMAKE_VERSION VERSION_GREATER 3.16)
    # libary output directories
    set_target_properties(${LIB_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set_target_properties(${LIB_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
    set_target_properties(${LIB_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
    set( ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set( ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
    set( ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
    #executable output directories
    set_target_properties(${LIB_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set_target_properties(${LIB_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
    set_target_properties(${LIB_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
    set_property(TARGET ${LIB_NAME} PROPERTY CXX_STANDARD 20)
endif()