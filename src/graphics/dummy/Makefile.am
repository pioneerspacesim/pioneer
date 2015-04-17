include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

AM_CPPFLAGS += -I$(srcdir)/../.. -isystem $(top_srcdir)/contrib

noinst_LIBRARIES = libgraphicsdummy.a
noinst_HEADERS = \
	RendererDummy.h \
	MaterialDummy.h \
	TextureDummy.h \
	RenderStateDummy.h \
	RenderTargetDummy.h \
	VertexBufferDummy.h

libgraphicsdummy_a_SOURCES = \
	RendererDummy.cpp
