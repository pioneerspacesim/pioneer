include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

AM_CPPFLAGS += -I$(srcdir)/../.. -isystem $(top_srcdir)/contrib
if !HAVE_LUA
AM_CPPFLAGS += -isystem @top_srcdir@/contrib/lua
endif

noinst_LIBRARIES = libgraphicsgl2.a
noinst_HEADERS = \
	GL2Renderer.h \
	GL2RenderTarget.h \
	GL2Program.h \
	GL2RingMaterial.h \
	GL2MultiMaterial.h \
	GL2ShieldMaterial.h \
	GL2SkyboxMaterial.h \
	GL2SphereImpostorMaterial.h \
	GL2StarfieldMaterial.h \
	GL2Material.h \
	GL2Texture.h \
	GL2GeoSphereMaterial.h \
	GL2UIMaterial.h \
	GL2GasGiantMaterial.h \
	GL2Uniform.h \
	GL2FresnelColourMaterial.h \
	GL2VertexBuffer.h \
	GL2Debug.h \
	GL2VtxColorMaterial.h \
	GL2RenderState.h \
	OpenGLLibs.h

libgraphicsgl2_a_SOURCES = \
	GL2VtxColorMaterial.cpp \
	GL2FresnelColourMaterial.cpp \
	GL2VertexBuffer.cpp \
	GL2GasGiantMaterial.cpp \
	GL2Uniform.cpp \
	GL2GeoSphereMaterial.cpp \
	GL2UIMaterial.cpp \
	GL2Material.cpp \
	GL2Texture.cpp \
	GL2MultiMaterial.cpp \
	GL2ShieldMaterial.cpp \
	GL2Program.cpp \
	GL2RingMaterial.cpp \
	GL2Renderer.cpp \
	GL2RenderTarget.cpp \
	GL2RenderState.cpp
