include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

INCLUDES = -I$(srcdir)/..

noinst_LIBRARIES = librender.a
noinst_HEADERS = \
	Render.h \
	Renderer.h \
	RendererGL2.h \
	RendererLegacy.h \
	RenderFrustum.h \
	RenderShader.h \
	RenderTarget.h

librender_a_SOURCES = \
	Render.cpp \
	Renderer.cpp \
	RendererGL2.cpp \
	RendererLegacy.cpp \
	RenderFrustum.cpp \
	RenderShader.cpp \
	RenderTarget.cpp
	

