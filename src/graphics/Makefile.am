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
	RenderTarget.h \
	Frustum.h \
	Light.h \
	Material.h \
	RenderState.h \
	VertexArray.h \
	Texture.h \
	TextureBuilder.h \
	Drawables.h \
	Types.h \
	VertexBuffer.h \
	opengl/gl_core_3_x.h \
	opengl/GLDebug.h \
	opengl/MaterialGL.h \
	opengl/RenderStateGL.h \
	opengl/RenderTargetGL.h \
	opengl/VertexBufferGL.h \
	opengl/FresnelColourMaterial.h \
	opengl/GasGiantMaterial.h \
	opengl/GeoSphereMaterial.h \
	opengl/MultiMaterial.h \
	opengl/Program.h \
	opengl/RendererGL.h \
	opengl/RingMaterial.h \
	opengl/ShieldMaterial.h \
	opengl/StarfieldMaterial.h \
	opengl/SkyboxMaterial.h \
	opengl/TextureGL.h \
	opengl/UIMaterial.h \
	opengl/Uniform.h \
	opengl/VtxColorMaterial.h

libgraphics_a_SOURCES = \
	Graphics.cpp \
	WindowSDL.cpp \
	Renderer.cpp \
	Frustum.cpp \
	Light.cpp \
	Material.cpp \
	VertexArray.cpp \
	TextureBuilder.cpp \
	Drawables.cpp \
	VertexBuffer.cpp \
	opengl/gl_core_3_x.c \
	opengl/MaterialGL.cpp \
	opengl/RenderStateGL.cpp \
	opengl/RenderTargetGL.cpp \
	opengl/VertexBufferGL.cpp \
	opengl/FresnelColourMaterial.cpp \
	opengl/GasGiantMaterial.cpp \
	opengl/GeoSphereMaterial.cpp \
	opengl/MultiMaterial.cpp \
	opengl/Program.cpp \
	opengl/ShieldMaterial.cpp \
	opengl/RendererGL.cpp \
	opengl/RingMaterial.cpp \
	opengl/TextureGL.cpp \
	opengl/UIMaterial.cpp \
	opengl/Uniform.cpp \
	opengl/VtxColorMaterial.cpp
