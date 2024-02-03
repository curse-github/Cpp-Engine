include(ExternalProject)
function(LoadEngine)# Loads includes and libraries for the engine
    if ("${ARGV0}" STREQUAL "64")
        set(GAME_ARCHITECTURE "x64")
    elseif("${ARGV0}" STREQUAL "32")
        set(GAME_ARCHITECTURE "Win32")
    endif()
    set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/bin/_deps")
    ExternalProject_Add(Engine
        PREFIX ${CMAKE_BINARY_DIR}/bin/_deps
        GIT_REPOSITORY https://github.com/curse-github/Cpp-Engine.git
        GIT_TAG        master
        DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/bin/_deps/Engine"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/bin/_deps/Engine"
        BINARY_DIR "${CMAKE_BINARY_DIR}/bin/_deps/Engine-build"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/bin/_deps/Engine-lib"
        PREFIX "${CMAKE_BINARY_DIR}/bin/_deps/Engine-lib"
        STAMP_DIR "${CMAKE_BINARY_DIR}/bin/_deps/Engine-stamp"
        TMP_DIR "${CMAKE_BINARY_DIR}/bin/_deps/Engine-tmp"
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -A ${GAME_ARCHITECTURE} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -S ${CMAKE_BINARY_DIR}/bin/_deps/Engine -B ${CMAKE_BINARY_DIR}/bin/_deps/Engine-build
        BUILD_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/bin/_deps/Engine-build
        INSTALL_COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR}/bin/_deps/Engine-build --prefix ${CMAKE_BINARY_DIR}/bin/_deps/Engine-lib
        TEST_COMMAND ""
    )
    add_dependencies(${EXEC_NAME} Engine)
    MESSAGE(${CMAKE_BINARY_DIR}/bin/_deps/Engine-lib/include)
    target_include_directories(${EXEC_NAME} PRIVATE ${CMAKE_BINARY_DIR}/bin/_deps/Engine-lib/includes)
    
    file(GLOB LIBS ${CMAKE_BINARY_DIR}/bin/_deps/Engine-lib/lib/*.lib)
    target_link_libraries(${EXEC_NAME} ${LIBS})
endfunction()