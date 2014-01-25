include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

AM_CPPFLAGS += -I$(srcdir)/..

noinst_LIBRARIES = libscenegraph.a
noinst_HEADERS = \
	AnimationChannel.h \
	Animation.h \
	AnimationKey.h \
	BaseLoader.h \
	Billboard.h \
	BinaryConverter.h \
	CollisionGeometry.h \
	CollisionVisitor.h \
	ColorMap.h \
	DumpVisitor.h \
	FindNodeVisitor.h \
	Group.h \
	Label3D.h \
	LoaderDefinitions.h \
	Loader.h \
	LOD.h \
	MatrixTransform.h \
	ModelNode.h \
	SceneGraph.h \
	Model.h \
	ModelSkin.h \
	Node.h \
	NodeCopyCache.h \
	NodeVisitor.h \
	Parser.h \
	Pattern.h \
	StaticGeometry.h \
	Thruster.h \
	Lua.h

libscenegraph_a_SOURCES = \
	Animation.cpp \
	BaseLoader.cpp \
	Billboard.cpp \
	BinaryConverter.cpp \
	CollisionGeometry.cpp \
	CollisionVisitor.cpp \
	ColorMap.cpp \
	DumpVisitor.cpp \
	FindNodeVisitor.cpp \
	Group.cpp \
	Label3D.cpp \
	Loader.cpp \
	LOD.cpp \
	MatrixTransform.cpp \
	ModelNode.cpp \
	Model.cpp \
	ModelSkin.cpp \
	Node.cpp \
	NodeVisitor.cpp \
	Parser.cpp \
	Pattern.cpp \
	StaticGeometry.cpp \
	Thruster.cpp \
	Lua.cpp \
	LuaModel.cpp \
	LuaModelSkin.cpp

AM_CPPFLAGS += -isystem @top_srcdir@/contrib
if !HAVE_LUA
AM_CPPFLAGS += -isystem @top_srcdir@/contrib/lua
endif
