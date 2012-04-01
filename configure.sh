#!/bin/sh
if [ -d "build" ]; then
	echo "Warning: directory 'build' already exists."
	#exit 0
else
	mkdir build
fi
cd build
# Note: we could include here some options on the command line, if necessary
cmake ..

