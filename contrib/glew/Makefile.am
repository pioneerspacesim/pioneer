#  Process this file with automake to produce Makefile.in

if !HAVE_GLEW

include $(top_srcdir)/Makefile.common

AM_CPPFLAGS += -I @top_srcdir@/contrib/glew/GL

noinst_LIBRARIES = libglew.a
libglew_a_SOURCES = glew.c
noinst_HEADERS = GL/glew.h GL/glxew.h GL/wglew.h GL/eglew.h

endif
