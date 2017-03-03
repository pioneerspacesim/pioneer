#  Process this file with automake to produce Makefile.in
include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(SDL2_CFLAGS)

AM_CPPFLAGS += -I$(top_srcdir)/contrib/imgui -I$(top_srcdir)/contrib/imgui/examples/libs/gl3w/ -I$(top_srcdir)/contrib/ -I$(top_srcdir)/src/
noinst_LIBRARIES = libimgui.a
libimgui_a_SOURCES = imgui.cpp imgui_draw.cpp examples/sdl_opengl3_example/imgui_impl_sdl_gl3.cpp examples/sdl_opengl2_example/imgui_impl_sdl.cpp
noinst_HEADERS = imgui.h examples/sdl_opengl3_example/imgui_impl_sdl_gl3.h examples/sdl_opengl2_example/imgui_impl_sdl.h
