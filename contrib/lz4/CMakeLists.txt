cmake_minimum_required(VERSION 3.5)
project(lz4 LANGUAGES C)

list(APPEND LZ4_SOURCES
    lz4.c
    lz4hc.c
    lz4frame.c
    xxhash.c
)

add_library(lz4 STATIC ${LZ4_SOURCES})

target_include_directories(lz4 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(lz4 PRIVATE "-O3")
# target_compile_definitions(lz4 PRIVATE "XXH_NAMESPACE=LZ4_")
