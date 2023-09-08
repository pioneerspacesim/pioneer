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
set(ASSIMP_LIBRARIES optimized ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/assimp-vc142-mt.lib debug ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/assimp-vc142-mtd.lib)

set(VORBISFILE_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/include)
set(VORBISFILE_LIBRARIES ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/ogg.lib;${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/vorbis.lib;${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/vorbisfile.lib)

set(SIGCPP_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/include/sigc++-2.0)
set(SIGCPP_LIBRARIES optimized ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/sigc-vc140-2_0.lib debug ${CMAKE_SOURCE_DIR}/../pioneer-thirdparty/win32/lib/${MSVC_ARCH}/vs2019/sigc-vc140-d-2_0.lib)
