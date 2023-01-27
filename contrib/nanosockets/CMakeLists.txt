cmake_minimum_required(VERSION 3.1)
project(nanosockets C)

if (MSYS OR MINGW)
    set(CMAKE_C_FLAGS "-static")

    add_definitions(-DWINVER=0x0601)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_library(nanosockets STATIC nanosockets.c ${SOURCES})

if (NOT UNIX)
	target_link_libraries(nanosockets ws2_32)
	SET_TARGET_PROPERTIES(nanosockets PROPERTIES PREFIX "")
endif()
