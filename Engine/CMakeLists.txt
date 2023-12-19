function(LoadDefault)# Loads default includes and libraries
    cmake_policy(SET CMP0072 NEW)
    find_package(OpenGL REQUIRED)
    add_subdirectory(imports/dependencies/glad/ glad)
    target_link_libraries(${EXEC_NAME} PRIVATE
        OpenGL::GL
        glad
    )# Link libraries to executable.
    target_include_directories(${EXEC_NAME} PUBLIC imports/include)# Include directories.
endfunction()
function(LoadX86)# Loads x86 includes and libraries
    target_include_directories(${EXEC_NAME} PUBLIC imports/include32 Headers)# Include directories.
    target_link_libraries(${EXEC_NAME} PRIVATE
        imports/dependencies32/freetype
        imports/dependencies32/glfw-3.3.9.bin.WIN/lib-vc2022/glfw3
        imports/dependencies32/glfw-3.3.9.bin.WIN/lib-vc2022/glfw3dll
    )# Link libraries to executable.
endfunction()
function(LoadX64)# Loads x64 includes and libraries
    target_include_directories(${EXEC_NAME} PUBLIC imports/include64 Headers)# Include directories.
    target_link_libraries(${EXEC_NAME} PRIVATE
        imports/dependencies64/freetype
        imports/dependencies64/glfw-3.3.9.bin.WIN/lib-vc2022/glfw3
        imports/dependencies64/glfw-3.3.9.bin.WIN/lib-vc2022/glfw3dll
    )# Link libraries to executable.
endfunction()


# Set minimum required CMake version.
cmake_minimum_required(VERSION 3.16)
# Make cMake be quiet
set(CMAKE_INSTALL_MESSAGE NEVER)
# Set visual studio to only have release and debug configurations
set(CMAKE_CONFIGURATION_TYPES Release Debug)
# Set project name.
project(Engine# ${PROJECT_NAME}
    VERSION 0.5
    LANGUAGES CXX
)
# Add source files.
set(SOURCES
    Src/Lib.cpp
    Src/Engine.cpp
    Src/Json.cpp
    Src/Main.cpp
)
# Add header files.
set(HEADERS
    Headers/Lib.h
    Headers/Engine.h
    Headers/Json.h
    Headers/Main.h
)
# Add executable.
set(EXEC_NAME EngineExec)
add_executable(${EXEC_NAME} ${HEADERS} ${SOURCES})
if (WIN32)
    MESSAGE("OS detected: Windows")
    if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")# is 64 bit windows
        MESSAGE("Architecture detected: x64")
        LoadDefault()
        loadX64()
    elseif ("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "WIN32")# us 32 bit windows
        MESSAGE("Architecture detected: x86")
        LoadDefault()
        loadX86()
    else()# Architecture could easily determined, try to detect a different way
        set(_x64processors AMD64 IA64 ARM64 EM64T)
        list (FIND _x64processors "${CMAKE_SYSTEM_PROCESSOR}" _index)
        if (${_index} GREATER -1)# is 64 bit windows
            MESSAGE("Architecture detected: x64")
            LoadDefault()
            loadX64()
        elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86")# us 32 bit windows
            MESSAGE("Architecture detected: x86")
            LoadDefault()
            loadX86()
        else()
            MESSAGE("Architecture could not be determined, build failed")
        endif()
    endif()
endif()