include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

INCLUDES = -I$(srcdir)/..

noinst_LIBRARIES = libnewmodel.a
noinst_HEADERS = \
	CollisionVisitor.h \
	DrawVisitor.h \
	Group.h \
	Loader.h \
	Newmodel.h \
	NModel.h \
	Node.h \
	NodeVisitor.h \
	StaticGeometry.h
	

libnewmodel_a_SOURCES = \
	CollisionVisitor.cpp \
	DrawVisitor.cpp \
	Group.cpp \
	Loader.cpp \
	NModel.cpp \
	Node.cpp \
	NodeVisitor.cpp \
	StaticGeometry.cpp

INCLUDES += -isystem @top_srcdir@/contrib
if !HAVE_LUA
INCLUDES += -isystem @top_srcdir@/contrib/lua
endif
