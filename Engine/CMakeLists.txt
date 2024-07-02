# CMakeList.txt : CMake project for Engine, include source and define
set(EXTERNALS ${CMAKE_SOURCE_DIR}/Engine/externals)
set(PROJECT_NAME EngineProj)
set(LIB_NAME Engine)

function(import_library)
    target_link_libraries(${LIB_NAME} ${ARGV0})
    install(FILES ${ARGV0} DESTINATION lib)
endfunction()
function(LoadDefault)# Loads default includes and libraries
    target_include_directories(${LIB_NAME} PRIVATE ${EXTERNALS}/include)
    target_include_directories(${LIB_NAME} PRIVATE Headers)
    install(DIRECTORY ${EXTERNALS}/include/glad DESTINATION includes)
    install(DIRECTORY ${EXTERNALS}/include/KHR DESTINATION includes)
    install(DIRECTORY Headers/ DESTINATION includes/Engine)
    # Link libraries.
    cmake_policy(SET CMP0072 NEW)
    find_package(OpenGL REQUIRED)
    #add_subdirectory(${EXTERNALS}/dependencies/glad/ glad)
    #target_link_libraries(${LIB_NAME}
    #    OpenGL::GL
    #    glad
    #)
    #install(FILES ${CMAKE_BINARY_DIR}/Engine/glad/${CMAKE_BUILD_TYPE}/glad.lib DESTINATION lib)
endfunction()
function(LoadArchitectureSpecific)# Loads includes and libraries specific to 64 bit or 32 bit
    target_include_directories(${LIB_NAME} PRIVATE ${EXTERNALS}/include${ARGV0})
    install(DIRECTORY ${EXTERNALS}/include${ARGV0}/GLFW DESTINATION includes)
    target_link_libraries(${LIB_NAME} ${EXTERNALS}/dependencies${ARGV0}/portaudio.lib)
    import_library(${EXTERNALS}/dependencies${ARGV0}/freetyped.lib)
    import_library(${EXTERNALS}/dependencies${ARGV0}/glfw-3.3.9.bin.WIN/lib-vc2022/glfw3.lib)
    import_library(${EXTERNALS}/dependencies${ARGV0}/glfw-3.3.9.bin.WIN/lib-vc2022/glfw3dll.lib)
    import_library(${EXTERNALS}/dependencies${ARGV0}/portaudio.lib)
    import_library(${EXTERNALS}/dependencies${ARGV0}/sndfile.lib)

    install(FILES ${EXTERNALS}/include/sndfile.h DESTINATION includes)
    install(FILES ${EXTERNALS}/include/sndfile.hh DESTINATION includes)
    file(GLOB PaHeaders ${EXTERNALS}/include/pa_*.h)
    install(FILES ${PaHeaders} DESTINATION includes/PA)
    install(FILES ${EXTERNALS}/include/portaudio.h DESTINATION includes/PA)

    if ("${ARGV0}" STREQUAL "64")
        set(PORTAUDIO_ARCHITECTURE "x64")
    elseif("${ARGV0}" STREQUAL "32")
        set(PORTAUDIO_ARCHITECTURE "x86")
    endif()
    install(FILES ${EXTERNALS}/dependencies${ARGV0}/portaudio_${PORTAUDIO_ARCHITECTURE}.dll DESTINATION dll)
    install(FILES ${EXTERNALS}/dependencies${ARGV0}/freetyped.lib DESTINATION dll)
    install(FILES ${EXTERNALS}/dependencies${ARGV0}/glfw-3.3.9.bin.WIN/lib-vc2022/glfw3.lib DESTINATION dll)
    install(FILES ${EXTERNALS}/dependencies${ARGV0}/glfw-3.3.9.bin.WIN/lib-vc2022/glfw3dll.lib DESTINATION dll)
    install(FILES ${EXTERNALS}/dependencies${ARGV0}/sndfile.lib DESTINATION dll)
endfunction()
function(installSelf)
    install(TARGETS ${LIB_NAME} INCLUDES DESTINATION lib )
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
# Add executable.
file(GLOB HEADERS "${CMAKE_SOURCE_DIR}/Engine/Headers/*.h")
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/Engine/*.cpp")
add_library(${LIB_NAME} ${HEADERS} ${SOURCES} ${EXTERNALS}/dependencies/glad.c)
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    target_compile_definitions(${LIB_NAME} PRIVATE _ENGINE_DEBUG=1)
endif()
MESSAGE("[Engine]: Detecting OS.")
if (WIN32)
    MESSAGE("[Engine]: OS detected: Windows")
    MESSAGE("[Engine]: Detecting Architecture.")
    if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")# is 64 bit windows
        MESSAGE("[Engine]: Architecture detected: 64 bit")
        LoadDefault()
        LoadArchitectureSpecific(64)
            installSelf()
    elseif ("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "WIN32" OR "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win32")# is 32 bit windows
        MESSAGE("[Engine]: Architecture detected: 32 bit")
        LoadDefault()
        LoadArchitectureSpecific(32)
            installSelf()
    else()# Architecture could easily determined, try to detect a different way
        set(_x64processors AMD64 IA64 ARM64 EM64T)
        list (FIND _x64processors "${CMAKE_SYSTEM_PROCESSOR}" _index)
        if (${_index} GREATER -1)# is 64 bit windows
            MESSAGE("[Engine]: Architecture detected: 64 bit")
            LoadDefault()
            LoadArchitectureSpecific(64)
            installSelf()
        elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86")# is 32 bit windows
            MESSAGE("[Engine]: Architecture detected: 32 bit")
            LoadDefault()
            LoadArchitectureSpecific(32)
            installSelf()
        else()
            MESSAGE("[Engine]: Architecture could not be determined, build failed")
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