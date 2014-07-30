include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

AM_CPPFLAGS += -I$(srcdir)/.. -isystem $(top_srcdir)/contrib

noinst_LIBRARIES = libgraphics.a
noinst_HEADERS = \
	Graphics.h \
	WindowSDL.h \
	Renderer.h \
	RendererGL2.h \
	RenderTarget.h \
	GLDebug.h \
	Frustum.h \
	Light.h \
	Material.h \
	RenderState.h \
	VertexArray.h \
	Texture.h \
	TextureGL.h \
	TextureBuilder.h \
	Drawables.h \
	Types.h \
	VertexBuffer.h \
	gl2/GL2Material.h \
	gl2/GL2RenderState.h \
	gl2/GL2RenderTarget.h \
	gl2/GL2VertexBuffer.h \
	gl2/FresnelColourMaterial.h \
	gl2/GasGiantMaterial.h \
	gl2/GeoSphereMaterial.h \
	gl2/MultiMaterial.h \
	gl2/Program.h \
	gl2/RingMaterial.h \
	gl2/ShieldMaterial.h \
	gl2/StarfieldMaterial.h \
	gl2/SkyboxMaterial.h \
	gl2/Uniform.h

libgraphics_a_SOURCES = \
	Graphics.cpp \
	WindowSDL.cpp \
	Renderer.cpp \
	RendererGL2.cpp \
	Frustum.cpp \
	Light.cpp \
	Material.cpp \
	VertexArray.cpp \
	TextureGL.cpp \
	TextureBuilder.cpp \
	Drawables.cpp \
	VertexBuffer.cpp \
	gl2/GL2Material.cpp \
	gl2/GL2RenderState.cpp \
	gl2/GL2RenderTarget.cpp \
	gl2/GL2VertexBuffer.cpp \
	gl2/FresnelColourMaterial.cpp \
	gl2/GasGiantMaterial.cpp \
	gl2/GeoSphereMaterial.cpp \
	gl2/MultiMaterial.cpp \
	gl2/Program.cpp \
	gl2/ShieldMaterial.cpp \
	gl2/RingMaterial.cpp \
	gl2/Uniform.cpp
