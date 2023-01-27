cmake_minimum_required(VERSION 2.6)
project(nanosockets C)

set(NANOSOCKETS_STATIC "0" CACHE BOOL "Create a static library")
set(NANOSOCKETS_SHARED "0" CACHE BOOL "Create a shared library")

if (MSYS OR MINGW)
    set(CMAKE_C_FLAGS "-static") 

    add_definitions(-DWINVER=0x0601)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

if (NANOSOCKETS_STATIC)
    add_library(nanosockets_static STATIC nanosockets.c ${SOURCES})

    if (NOT UNIX)
        target_link_libraries(nanosockets_static ws2_32)
        SET_TARGET_PROPERTIES(nanosockets_static PROPERTIES PREFIX "")
    endif()
endif()

if (NANOSOCKETS_SHARED)
    add_definitions(-DNANOSOCKETS_DLL)
    add_library(nanosockets SHARED nanosockets.c ${SOURCES})

    if (NOT UNIX)
        target_link_libraries(nanosockets ws2_32)
        SET_TARGET_PROPERTIES(nanosockets PROPERTIES PREFIX "")
    endif()
endif()
