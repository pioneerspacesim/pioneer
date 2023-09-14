#!/bin/bash

# Package a build and prepare it for upload to Github.

TAG_NAME=$(git describe HEAD)
if [ -z "$TAG_NAME" ]; then
	TAG_NAME=$(date +%Y%m%d)
fi

mkdir release

mv out/install/linux-x64-release "release/pioneer-linux-x64-$TAG_NAME"
cd release

tar -czf "pioneer-linux-x64-$TAG_NAME.tar.gz" "pioneer-linux-x64-$TAG_NAME"

if [ $? -ne 0 ]; then
	echo "Release failed!"
	exit 1
fi

echo "Release finished successfully!"

exit 0
