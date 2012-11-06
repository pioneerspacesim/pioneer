include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

INCLUDES = -I$(srcdir)/..

noinst_LIBRARIES = libtext.a
noinst_HEADERS = \
	FontDescriptor.h \
	Font.h \
	TextSupport.h \
	TextureFont.h \
	VectorFont.h

libtext_a_SOURCES = \
	FontDescriptor.cpp \
	Font.cpp \
	TextSupport.cpp \
	TextureFont.cpp \
	VectorFont.cpp
