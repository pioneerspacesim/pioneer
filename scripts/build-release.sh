#!/bin/bash

# Package a build and prepare it for upload to Github.

BINARIES=("build/pioneer" "build/modelcompiler" "build/savegamedump" "build/editor")
COPY_DIR=release

# Append .exe to the binaries if we're building for windows.
if [ "$BUILD_TYPE" == "mxe" ]; then
    for i in ${!BINARIES[*]}; do
        BINARIES[$i]="${BINARIES[$i]}.exe"
    done
fi

mkdir -p $COPY_DIR

echo "Starting release process..."

# Copy binaries
cp "${BINARIES[@]}" $COPY_DIR
if [ "$BUILD_TYPE" == "mxe" ]; then
    cp src/pioneer.map src/modelcompiler.map $COPY_DIR
fi

echo "Copied binaries."

# Copy the text files.
cp AUTHORS.txt $COPY_DIR
cp Changelog.txt $COPY_DIR
cp Modelviewer.txt $COPY_DIR
cp Quickstart.txt $COPY_DIR
cp README.md $COPY_DIR

# Copy the licenses
cp -R licenses $COPY_DIR

echo "Copied text files."

# Copy the text files
cp -R data $COPY_DIR

echo "Copied data files."

# Clean up the left-over build files.
find $COPY_DIR/data/lang -name cmn.json -delete
find $COPY_DIR/data '(' -name .gitignore -o -name Makefile\* -o -name CMakeLists.txt ')' -delete

# Clean up the (unneeded) source files for models.
find $COPY_DIR/data '(' -name \*.dae -o -name \*.obj ')' -delete

echo "Cleaned up remaining build files."

mkdir -p release/zip

echo "Bundling output..."

TAG_NAME=$(git describe --tags --exact-match HEAD || date +%Y%m%d)

if [ "$BUILD_TYPE" == "mxe" ]; then
    zip -r "release/zip/pioneer-$TAG_NAME-mxe.zip" release/* -x *release/zip*
else
    tar -czf "release/zip/pioneer-$TAG_NAME.tar.gz" --exclude=release/zip release/*
fi

if [ $? -ne 0 ]; then
	echo "Release failed!"
	exit 1
fi

echo "Release finished successfully!"

exit 0
