include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

INCLUDES = -I$(srcdir)/..

noinst_LIBRARIES = libnewmodel.a
noinst_HEADERS = \
	Animation.h \
	AnimationChannel.h \
	AnimationKey.h \
	Billboard.h \
	CollisionVisitor.h \
	ColorMap.h \
	DrawVisitor.h \
	Group.h \
	LOD.h \
	Loader.h \
	MatrixTransform.h \
	ModelNode.h \
	Newmodel.h \
	NModel.h \
	Node.h \
	NodeVisitor.h \
	Parser.h \
	StaticGeometry.h \
	Thruster.h
	

libnewmodel_a_SOURCES = \
	Animation.cpp \
	Billboard.cpp \
	CollisionVisitor.cpp \
	ColorMap.cpp \
	DrawVisitor.cpp \
	Group.cpp \
	Loader.cpp \
	LOD.cpp \
	MatrixTransform.cpp \
	ModelNode.cpp \
	NModel.cpp \
	Node.cpp \
	NodeVisitor.cpp \
	Parser.cpp \
	StaticGeometry.cpp \
	Thruster.cpp

INCLUDES += -isystem @top_srcdir@/contrib
if !HAVE_LUA
INCLUDES += -isystem @top_srcdir@/contrib/lua
endif
