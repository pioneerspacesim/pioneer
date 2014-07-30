#  Process this file with automake to produce Makefile.in

include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

AM_CPPFLAGS += -I$(srcdir)/.. -isystem $(top_srcdir)/contrib

noinst_LIBRARIES = libcollider.a
libcollider_a_SOURCES = \
	BVHTree.cpp \
	CollisionSpace.cpp \
	Geom.cpp \
	GeomTree.cpp

noinst_HEADERS = \
	BVHTree.h \
	CollisionContact.h \
	CollisionSpace.h \
	Geom.h \
	GeomTree.h \
	collider.h
