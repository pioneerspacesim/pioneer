#! /bin/bash

if [[ -z "$1" ]]; then
    echo "please mention the path to appimage as follows:"
    echo "pioneer_appimage.sh /path/to/linuxdeploy-x86_64.AppImage"
    exit 2
fi
LINUX_DEPLOY_PATH=$1
shift


CMAKE_OPTS=("-DCMAKE_EXPORT_COMPILE_COMMANDS=1")
CMAKE_OPTS+=("-DPIONEER_DATA_DIR=usr/share/data")
CMAKE_OPTS+=("-DCMAKE_INSTALL_PREFIX=./appdir/usr")
CMAKE_OPTS+=("-DPIONEER_INSTALL_DATADIR=share")
CMAKE_OPTS+=("-DAPPIMAGE_BUILD=1")
if [ "$1" = "thirdparty" ]; then
	shift 1
	cd pioneer-thirdparty/ && autoconf && ./configure && make assimp && make sdl2 && make sdl2_image && cd ../
	CMAKE_OPTS+=("-DUSE_PIONEER_THIRDPARTY=1")
elif [ "$1" = "cmake" ]; then
	shift 1
fi

CMAKE_OPTS+=("$@")

mkdir -p build; cd build
cmake .. "${CMAKE_OPTS[@]}"

make all build-data -j4
make install
ARCH=x86_64 $LINUX_DEPLOY_PATH --appdir appdir --output appimage
