include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

AM_CPPFLAGS += -I$(srcdir)/../.. -isystem $(top_srcdir)/contrib

noinst_LIBRARIES = libgraphicsgl21.a
noinst_HEADERS = \
	gl_core_2_1.hpp \
	OpenGLLibs.h \
	GL2Debug.h \
	GL2Material.h \
	GL2RenderState.h \
	GL2RenderTarget.h \
	GL2VertexBuffer.h \
	GL2FresnelColourMaterial.h \
	GL2GasGiantMaterial.h \
	GL2GenGasGiantColourMaterial.h \
	GL2GeoSphereMaterial.h \
	GL2MultiMaterial.h \
	GL2Program.h \
	GL2Renderer.h \
	GL2RingMaterial.h \
	GL2ShieldMaterial.h \
	GL2StarfieldMaterial.h \
	GL2SkyboxMaterial.h \
	GL2Texture.h \
	GL2UIMaterial.h \
	GL2Uniform.h \
	GL2VtxColorMaterial.h \
	GL2BillboardMaterial.h

libgraphicsgl21_a_SOURCES = \
	gl_core_2_1.cpp \
	GL2Material.cpp \
	GL2RenderState.cpp \
	GL2RenderTarget.cpp \
	GL2VertexBuffer.cpp \
	GL2FresnelColourMaterial.cpp \
	GL2GasGiantMaterial.cpp \
	GL2GenGasGiantColourMaterial.cpp \
	GL2GeoSphereMaterial.cpp \
	GL2MultiMaterial.cpp \
	GL2Program.cpp \
	GL2ShieldMaterial.cpp \
	GL2Renderer.cpp \
	GL2RingMaterial.cpp \
	GL2Texture.cpp \
	GL2UIMaterial.cpp \
	GL2Uniform.cpp \
	GL2VtxColorMaterial.cpp \
	GL2BillboardMaterial.cpp
