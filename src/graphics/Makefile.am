include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

INCLUDES = -I$(srcdir)/..

noinst_LIBRARIES = libgraphics.a
noinst_HEADERS = \
	Graphics.h \
	Renderer.h \
	RendererGL2.h \
	RendererLegacy.h \
	Frustum.h \
	Shader.h \
	RenderTarget.h \
	Material.h \
	StaticMesh.h \
	Surface.h \
	VertexArray.h \
	Drawables.h

libgraphics_a_SOURCES = \
	Graphics.cpp \
	Renderer.cpp \
	RendererGL2.cpp \
	RendererLegacy.cpp \
	Frustum.cpp \
	Shader.cpp \
	RenderTarget.cpp \
	Material.cpp \
	StaticMesh.cpp \
	VertexArray.cpp \
	Drawables.cpp
