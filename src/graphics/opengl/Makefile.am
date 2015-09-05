include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

AM_CPPFLAGS += -I$(srcdir)/../.. -isystem $(top_srcdir)/contrib

noinst_LIBRARIES = libgraphicsopengl.a
noinst_HEADERS = \
	gl_core_3_x.h \
	GLDebug.h \
	MaterialGL.h \
	RenderStateGL.h \
	RenderTargetGL.h \
	VertexBufferGL.h \
	FresnelColourMaterial.h \
	GasGiantMaterial.h \
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
	VtxColorMaterial.h

libgraphicsopengl_a_SOURCES = \
	gl_core_3_x.c \
	MaterialGL.cpp \
	RenderStateGL.cpp \
	RenderTargetGL.cpp \
	VertexBufferGL.cpp \
	FresnelColourMaterial.cpp \
	GasGiantMaterial.cpp \
	GeoSphereMaterial.cpp \
	MultiMaterial.cpp \
	Program.cpp \
	ShieldMaterial.cpp \
	RendererGL.cpp \
	RingMaterial.cpp \
	TextureGL.cpp \
	UIMaterial.cpp \
	Uniform.cpp \
	VtxColorMaterial.cpp
