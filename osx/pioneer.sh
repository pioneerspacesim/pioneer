#!/bin/sh
PWD=`dirname "${0}"`
OSREV=`uname -r | cut -d. -f1`
if [ "$OSREV" -ge 11 ] ; then
	export DYLD_LIBRARY_PATH=${PWD}/libs
	export DYLD_FRAMEWORK_PATH=${PWD}/libs
else
	export DYLD_FALLBACK_LIBRARY_PATH=${PWD}/libs
	export DYLD_FALLBACK_FRAMEWORK_PATH=${PWD}/libs
fi
cd "${PWD}"; ./pioneer.osx $1 $2 $3
