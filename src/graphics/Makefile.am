SUBDIRS = opengl dummy

include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

AM_CPPFLAGS += -I$(srcdir)/.. -isystem $(top_srcdir)/contrib
if !HAVE_LUA
AM_CPPFLAGS += -isystem @top_srcdir@/contrib/lua
endif

noinst_LIBRARIES = libgraphics.a
noinst_HEADERS = \
	Graphics.h \
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
	Stats.h \
	VertexBuffer.h

libgraphics_a_SOURCES = \
	Graphics.cpp \
	Renderer.cpp \
	Frustum.cpp \
	Light.cpp \
	Material.cpp \
	VertexArray.cpp \
	TextureBuilder.cpp \
	Drawables.cpp \
	Stats.cpp \
	VertexBuffer.cpp
