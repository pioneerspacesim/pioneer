#!/bin/bash
set -e

# Package a build and prepare it for upload to Github.

TAG_NAME=$(git describe --tags --exact-match HEAD || date +%Y%m%d)

if [ -z "$BUILD_SLUG" ]; then
	BUILD_SLUG=linux-x64-release
fi

mkdir release

mv out/install/pioneer-$BUILD_SLUG "release/pioneer-linux-x64-$TAG_NAME"
cd release

tar -czf "pioneer-linux-x64-$TAG_NAME.tar.gz" "pioneer-linux-x64-$TAG_NAME"

if [ $? -ne 0 ]; then
	echo "Release failed!"
	exit 1
fi

echo "Release finished successfully!"

exit 0
