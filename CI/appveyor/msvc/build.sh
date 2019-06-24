#!/c/msys64/usr/bin/bash -l

# Exit immediately upon error
set -e

# Echo the commands
set +v

mkdir -p /c/projects/pioneer/build
cd /c/projects/pioneer/build

cmake -G 'Visual Studio 15 Win64' \
	-DCMAKE_INSTALL_PREFIX="/c/Program Files/Pioneer" \
	-DPIONEER_DATA_DIR="/c/Program Files/Pioneer/data" \
	-DCMAKE_BUILD_TYPE:STRING=Release \
	-DGIT_EXECUTABLE=/c/Program\\ Files/Git/cmd/git.exe \
	/c/projects/pioneer

cmake --build . --config Release --target pioneer
