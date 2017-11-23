include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

AM_CPPFLAGS += -I$(srcdir)/../.. -isystem $(top_srcdir)/contrib
if !HAVE_LUA
AM_CPPFLAGS += -isystem @top_srcdir@/contrib/lua
endif

noinst_LIBRARIES = libgraphicsopengl.a
noinst_HEADERS = \
	OpenGLLibs.h \
	GLDebug.h \
	MaterialGL.h \
	RenderStateGL.h \
	RenderTargetGL.h \
	VertexBufferGL.h \
	FresnelColourMaterial.h \
	GasGiantMaterial.h \
	GenGasGiantColourMaterial.h \
	GeoSphereMaterial.h \
	MultiMaterial.h \
	Program.h \
	RendererGL.h \
	RingMaterial.h \
	ShieldMaterial.h \
	StarfieldMaterial.h \
	SkyboxMaterial.h \
	TextureGL.h \
	UIMaterial.h \
	Uniform.h \
	VtxColorMaterial.h \
	BillboardMaterial.h

libgraphicsopengl_a_SOURCES = \
	MaterialGL.cpp \
	RenderStateGL.cpp \
	RenderTargetGL.cpp \
	VertexBufferGL.cpp \
	FresnelColourMaterial.cpp \
	GasGiantMaterial.cpp \
	GenGasGiantColourMaterial.cpp \
	GeoSphereMaterial.cpp \
	MultiMaterial.cpp \
	Program.cpp \
	ShieldMaterial.cpp \
	RendererGL.cpp \
	RingMaterial.cpp \
	TextureGL.cpp \
	UIMaterial.cpp \
	Uniform.cpp \
	VtxColorMaterial.cpp \
	BillboardMaterial.cpp
