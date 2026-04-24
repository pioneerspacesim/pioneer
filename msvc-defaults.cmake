if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(MSVC_ARCH x86)
else()
    set(MSVC_ARCH x64)
endif()

set(FREETYPE_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/include/freetype)
set(FREETYPE_LIBRARY ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/freetype.lib)

set(SDL2_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/include)
set(SDL2_LIBRARIES ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/SDL2.lib;${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/SDL2main.lib)

set(SDL2_IMAGE_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/include)
set(SDL2_IMAGE_LIBRARIES ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/SDL2_image.lib)

set(ASSIMP_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/include)
set(ASSIMP_LIBRARIES optimized ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/assimp-vc143-mt.lib debug ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/assimp-vc143-mtd.lib)

set(VORBISFILE_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/include)
set(VORBISFILE_LIBRARIES ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/ogg.lib;${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/vorbis.lib;${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/vorbisfile.lib)

set(SIGCPP_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/include/sigc++-2.0)
set(SIGCPP_LIBRARIES optimized ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/sigc-vc140-2_0.lib debug ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/sigc-vc140-d-2_0.lib)

add_library(OpenAL SHARED IMPORTED)
add_library(OpenAL::OpenAL ALIAS OpenAL)
set_target_properties(OpenAL PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/bin/${MSVC_ARCH}/vs2019/OpenAL32.dll
    IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/libOpenAL32.dll.a
)
target_include_directories(OpenAL INTERFACE ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/include)
set(OpenAL_FOUND ON)
