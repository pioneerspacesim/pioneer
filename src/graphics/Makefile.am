SUBDIRS = opengl dummy

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
	Stats.h \
	VertexBuffer.h

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
	Stats.cpp \
	VertexBuffer.cpp
