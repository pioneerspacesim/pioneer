#  Process this file with automake to produce Makefile.in
include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(SDL2_CFLAGS)

AM_CPPFLAGS += -DIMGUI_IMPL_OPENGL_LOADER_GLEW=1 -I$(top_srcdir)/contrib/imgui -I$(top_srcdr)/contrib/glew -I$(top_srcdir)/contrib/imgui/examples/ -std=c++11
noinst_LIBRARIES = libimgui.a
libimgui_a_SOURCES = imgui.cpp imgui_draw.cpp imgui_widgets.cpp imgui_demo.cpp examples/imgui_impl_opengl3.cpp examples/imgui_impl_sdl.cpp
noinst_HEADERS = imgui.h examples/imgui_impl_opengl3.h examples/imgui_impl_opengl3.h examples/imgui_impl_sdl.h
