#  Process this file with automake to produce Makefile.in
include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(SDL2_CFLAGS)

CPPFLAGS+="-Iexamples/libs/gl3w/"
noinst_LIBRARIES = libimgui.a
libimgui_a_SOURCES = imgui.cpp imgui_draw.cpp examples/sdl_opengl3_example/imgui_impl_sdl_gl3.cpp
noinst_HEADERS = imgui.h examples/sdl_opengl3_example/imgui_impl_sdl_gl3.h
