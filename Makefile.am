## Process this file with automake to produce Makefile.in
SUBDIRS = contrib src data osx win32

include $(top_srcdir)/Makefile.common

.PHONY: codedoc
codedoc:
	@naturaldocs -i src/ -i data/libs/ -xi src/data/ -o HTML codedoc/ -p nd/ -do -ro -s Default Local

.PHONY: enums
enums:
	python scripts/scan_enums.py -o src/enum_table.cpp src/*.h

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
	Makefile.common \
	modelviewer \
	pioneer
