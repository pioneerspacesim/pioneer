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
	Light.h \
	Shader.h \
	Material.h \
	StaticMesh.h \
	Surface.h \
	VertexArray.h \
	Texture.h \
	TextureGL.h \
	TextureBuilder.h \
	Drawables.h

libgraphics_a_SOURCES = \
	Graphics.cpp \
	Renderer.cpp \
	RendererGL2.cpp \
	RendererLegacy.cpp \
	Frustum.cpp \
	Light.cpp \
	Shader.cpp \
	Material.cpp \
	StaticMesh.cpp \
	VertexArray.cpp \
	TextureGL.cpp \
	TextureBuilder.cpp \
	Drawables.cpp
