## Process this file with automake to produce Makefile.in
SUBDIRS = contrib $(PLATFORM_SUBDIRS) src data osx

PATH := @MXE_PATH@:$(PATH)

include $(top_srcdir)/Makefile.common

.PHONY: codedoc
codedoc:
	@mkdir -p codedoc
	@naturaldocs -i ${top_srcdir}/src/ -i ${top_srcdir}/data/libs/ -xi ${top_srcdir}/src/data/ -o HTML codedoc/ -p ${top_srcdir}/nd/ -do -ro -s Default Local

.PHONY: enums
enums:
	python scripts/scan_enums.py -r --pattern=*.h -o src/enum_table.cpp src

EXTRA_DIST = \
	AUTHORS.txt \
	COMPILING.OSX.txt \
	Changelog.txt \
	Modelviewer.txt \
	Quickstart.txt \
	README.txt \
	licenses/CC-BY-SA-3.0.txt \
	licenses/GPL-3.txt \
	licenses/SIL-1.1.txt \
	bootstrap \
	generic-exec \
	Makefile.common \
	modelviewer \
	pioneer
