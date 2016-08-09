include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

AM_CPPFLAGS += -I$(srcdir)/..

noinst_LIBRARIES = libimgui.a
noinst_HEADERS = \
	imgui.h \
	imconfig.h \
	imgui_internal.h \
	examples/sdl_opengl3_example/imgui_impl_sdl_gl3.h

libimgui_a_SOURCES = \
	imgui.cpp \
	imgui_draw.cpp \
	imgui_demo.cpp \
	examples/sdl_opengl3_example/imgui_impl_sdl_gl3.cpp

AM_CPPFLAGS += -isystem @top_srcdir@/contrib
