include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

INCLUDES = -I$(srcdir)/.. -isystem $(top_srcdir)/contrib

noinst_LIBRARIES = libgraphics.a
noinst_HEADERS = \
	Graphics.h \
	Renderer.h \
	RendererGL2.h \
	RendererLegacy.h \
	RenderTarget.h \
	Frustum.h \
	Light.h \
	Material.h \
	MaterialLegacy.h \
	StaticMesh.h \
	Surface.h \
	VertexArray.h \
	Texture.h \
	TextureGL.h \
	TextureBuilder.h \
	Drawables.h \
	gl2/GL2Material.h \
	gl2/GL2RenderTarget.h \
	gl2/GeoSphereMaterial.h \
	gl2/MultiMaterial.h \
	gl2/Program.h \
	gl2/RingMaterial.h \
	gl2/StarfieldMaterial.h \
	gl2/Uniform.h

libgraphics_a_SOURCES = \
	Graphics.cpp \
	Renderer.cpp \
	RendererGL2.cpp \
	RendererLegacy.cpp \
	Frustum.cpp \
	Light.cpp \
	Material.cpp \
	MaterialLegacy.cpp \
	StaticMesh.cpp \
	VertexArray.cpp \
	TextureGL.cpp \
	TextureBuilder.cpp \
	Drawables.cpp \
	gl2/GL2Material.cpp \
	gl2/GL2RenderTarget.cpp \
	gl2/GeoSphereMaterial.cpp \
	gl2/MultiMaterial.cpp \
	gl2/Program.cpp \
	gl2/RingMaterial.cpp \
	gl2/Uniform.cpp
