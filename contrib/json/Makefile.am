#  Process this file with automake to produce Makefile.in
include $(top_srcdir)/Makefile.common

AM_CPPFLAGS += -I @top_srcdir@/contrib -std=c++11

noinst_LIBRARIES = libjson.a
libjson_a_SOURCES = jsoncpp.cpp JsonUtils.cpp
noinst_HEADERS = json.h JsonUtils.h
