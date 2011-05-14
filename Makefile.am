## Process this file with automake to produce Makefile.in
SUBDIRS = src data osx win32

.PHONY: codedoc
codedoc:
	@naturaldocs -i src/ -o HTML codedoc/ -p nd/ -do -s Default Local

EXTRA_DIST = \
	AUTHORS.txt \
	COMPILING.OSX.txt \
	COPYING.txt \
	Changelog.txt \
	Modelviewer.txt \
	Quickstart.txt \
	README.txt \
	bootstrap \
	generic-exec \
	modelviewer \
	pioneer
