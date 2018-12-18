#!/bin/bash

# Package a build and prepare it for upload via Travis.

BINARIES=("build/pioneer" "build/modelcompiler")
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
ls *.txt | grep -v -E '(COMPILING\.txt|SAVEBUMP\.txt|CMakeLists\.txt)' | xargs cp -t $COPY_DIR
cp README.md $COPY_DIR
# Copy the licenses
cp -r licenses -t $COPY_DIR

echo "Copied text files."

# Copy the text files
cp -r data -t $COPY_DIR

echo "Copied data files."

# Clean up the left-over build files.
find $COPY_DIR/data/lang -name cmn.json -delete
find $COPY_DIR/data '(' -name .gitignore -o -name Makefile\* -o -name CMakeLists.txt ')' -delete

# Clean up the (unneeded) source files for models.
find $COPY_DIR/data '(' -name \*.dae -o -name \*.obj ')' -delete

echo "Cleaned up remaining build files."

mkdir -p release/zip

echo "Bundling output..."

if [ "$BUILD_TYPE" == "mxe" ]; then
    zip -r "release/zip/pioneer-$TRAVIS_TAG-mxe.zip" release/* -x *release/zip*
else
    tar -caf "release/zip/pioneer-$TRAVIS_TAG.tar.gz" --exclude release/zip release/*
fi

echo "Release finished successfully!"

exit 0
