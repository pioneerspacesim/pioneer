include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

INCLUDES = -I$(srcdir)/..

noinst_LIBRARIES = librender.a
noinst_HEADERS = \
	Render.h \
	RenderFrustum.h \
	RenderShader.h \
	RenderTarget.h

librender_a_SOURCES = \
	Render.cpp \
	RenderFrustum.cpp \
	RenderShader.cpp \
	RenderTarget.cpp
	

