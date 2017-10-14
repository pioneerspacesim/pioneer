#!/bin/bash

# OSX bin-dist script
#
# It grabs the binary, data files and libs and bundles them all
# in cotton wool.

# Run ./scripts/build-osx.sh and it will configure, make and upload
# the osx binary to sourceforge.

PIONEER="/Users/Phil/dev/pioneer"
DIST="${PIONEER}/pioneer-osx"
DATE=`date +%Y%m%d`
BASEOUTFILE="pioneer-${DATE}-osx"
UPLOAD_DIR=philbywhizz,pioneerspacesim@frs.sf.net:/home/frs/project/p/pi/pioneerspacesim

cd $PIONEER

echo "=== Clean and do a new build ==="

make clean
./configure LDFLAGS="-L/System/Library/Frameworks/OpenGL.framework/Libraries -L/System/Library/Frameworks/GLUT.framework"
make all

echo "=== OSX Packaging ${BASEOUTFILE} ==="

test -d $DIST && rm -fr $DIST
mkdir $DIST

# copy the pioneer binary
cp -v $PIONEER/src/pioneer $DIST/pioneer.osx

# copy the text files
cp -v $PIONEER/*.txt $DIST/

# copy the licenses folder
cp -r $PIONEER/licenses $DIST/licenses

# copy the data folder
cp -r $PIONEER/data $DIST/data

# Copy over main libs
mkdir $DIST/libs
cp -v /usr/local/lib/libfreetype.6.dylib $DIST/libs
cp -v /usr/local/lib/libSDL2_image-2.0.0.dylib $DIST/libs
cp -v /usr/local/lib/libSDL2-2.0.0.dylib $DIST/libs
cp -v /usr/local/opt/libsigc++/lib/libsigc-2.0.0.dylib $DIST/libs
cp -v /usr/local/lib/libvorbisfile.3.dylib $DIST/libs
cp -v /usr/local/lib/libpng16.16.dylib $DIST/libs
cp -v /usr/local/lib/libjpeg.9.dylib $DIST/libs
cp -v /usr/local/lib/libassimp.4.dylib $DIST/libs
cp -v /usr/local/lib/libtiff.5.dylib $DIST/libs
cp -v /usr/local/lib/libwebp.7.dylib $DIST/libs
cp -v /usr/local/lib/libogg.0.dylib $DIST/libs
cp -v /usr/local/lib/libvorbis.0.dylib $DIST/libs

# Copy over the shell script
cp $PIONEER/osx/pioneer.sh $DIST/pioneer

# Now archive this all up
echo "=== Archiving ${BASEOUTFILE} ==="
/usr/bin/tar cf $BASEOUTFILE.tar pioneer-osx
/usr/bin/bzip2 $BASEOUTFILE.tar

# Testing
echo "=== Pausing for testing $BASEOUTFILE ==="
read -n 1 -p "<ENTER>"

# Uploading
echo "=== Uploading ${BASEOUTFILE} ==="
scp $BASEOUTFILE.tar.bz2 $UPLOAD_DIR

# Clean up
rm -fr $DIST
rm $BASEOUTFILE.tar.bz2

echo "=== All done ==="
