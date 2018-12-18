#!/c/msys64/usr/bin/bash -l

# Exit immediately upon error
set -e

export PATH=/mingw64/bin:$PATH

# Install dependencies
/c/msys64/usr/bin/pacman --noconfirm -Sy \
	mingw-w64-x86_64-cmake \
	mingw-w64-x86_64-SDL2 \
	mingw-w64-x86_64-SDL2_image \
	mingw-w64-x86_64-assimp \
	mingw-w64-x86_64-freetype \
	mingw-w64-x86_64-glew \
	mingw-w64-x86_64-libvorbis \
	mingw-w64-x86_64-libpng \
	mingw-w64-x86_64-libsigc++
