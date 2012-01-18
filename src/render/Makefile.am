include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

INCLUDES = -I$(srcdir)/..

noinst_LIBRARIES = librender.a
noinst_HEADERS = \
	Render.h \
	Renderer.h \
	RenderFrustum.h \
	RenderShader.h \
	RenderTexture.h \
	RenderTarget.h

librender_a_SOURCES = \
	Render.cpp \
	Renderer.cpp \
	RenderFrustum.cpp \
	RenderShader.cpp \
	RenderTexture.cpp \
	RenderTarget.cpp
	

