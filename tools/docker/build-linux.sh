#!/bin/bash
# runs INSIDE the docker container. builds the linux host packages + AppImage.
# invoked by build.sh via `docker run`, not meant to be run standalone on host.

#docker specific
export GIT_CONFIG_COUNT=1
export GIT_CONFIG_KEY_0=safe.directory
export GIT_CONFIG_VALUE_0=/src
export APPIMAGE_EXTRACT_AND_RUN=1

set -e

echo "---> Docker Build <---"
mkdir -p build/release/docker
cd build/release/docker
cmake -DCMAKE_BUILD_TYPE=Release ../../.. -DCMAKE_INSTALL_PREFIX=./AppDir/usr
make package -j$(nproc)
# no exit for failing appimage build.
set +e
make install -j$(nproc)
mkdir -p generated/icon
convert ../../../res/harmony-900-color-screen.png -resize 256x256^ -gravity center -extent 256x256 generated/icon/h900edit.png
wget -N https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget -N https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage linuxdeploy-plugin-qt-x86_64.AppImage
source generated/version.env
export QMAKE=/usr/bin/qmake6
./linuxdeploy-x86_64.AppImage --appdir AppDir -i generated/icon/h900edit.png -d ../../../res/h900edit.desktop --plugin qt --output appimage
