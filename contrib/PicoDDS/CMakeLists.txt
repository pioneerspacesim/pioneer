project(picodds LANGUAGES CXX)

set(BUILD_SHARED_LIBS OFF)

add_library(${PROJECT_NAME} PicoDDS.cpp)
