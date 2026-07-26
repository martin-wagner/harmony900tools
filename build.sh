#!/bin/bash

# create build dirs
echo "---> Cleanup <---"
rm -rf build/
rm -rf out/
mkdir -p build/release/host
mkdir -p build/release/x86-64-windows
mkdir -p build/test/host

echo "---> Unit Tests <---"
cd build/test/host
cmake -DCMAKE_BUILD_TYPE=Test ../../.. || exit 1
make -j${nproc} && ctest --output-on-failure || exit 1

echo "---> Host Build <---"
cd ../../../build/release/host
cmake -DCMAKE_BUILD_TYPE=Release ../../.. -DCMAKE_INSTALL_PREFIX=./AppDir/usr || exit 1
make package -j${nproc} || exit 1
# no exit for failing appimage build.
make install -j${nproc}
mkdir -p generated/icon
convert ../../../res/harmony-900-color-screen.png  -resize 256x256^  -gravity center   -extent 256x256  generated/icon/h900edit.png
wget -N https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget -N https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage linuxdeploy-plugin-qt-x86_64.AppImage
source generated/version.env
export QMAKE=/usr/bin/qmake6
./linuxdeploy-x86_64.AppImage  --appdir AppDir -i generated/icon/h900edit.png -d ../../../res/h900edit.desktop --plugin qt --output appimage

echo "---> MXE Build <---"
cd ../../../build/release/x86-64-windows
export PATH=/opt/mxe/usr/bin:$PATH
x86_64-w64-mingw32.shared-cmake -DCMAKE_BUILD_TYPE=Release ../../.. -DCMAKE_INSTALL_PREFIX=./install || exit 1
make package -j${nproc} || exit 1

echo "---> collecting build products <---"
cd ../../..
mkdir out
cp build/release/host/h900tools*.tar.gz out
cp build/release/host/h900tools*.deb out
cp build/release/host/H900*.AppImage out
cp build/release/x86-64-windows/h900tools*.exe out
cp build/release/x86-64-windows/h900tools*.zip out
ls -lh out

echo "---< done >---"
