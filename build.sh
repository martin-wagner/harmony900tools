#!/bin/bash

# create build dirs
echo "---> Cleanup <---"
rm -rf build/
rm -rf out/
mkdir -p build/release/docker
mkdir -p build/release/x86-64-windows
mkdir -p build/test/host

echo "---> Unit Tests <---"
cd build/test/host
cmake -DCMAKE_BUILD_TYPE=Test ../../.. || exit 1
make -j$(nproc) && ctest --output-on-failure || exit 1
cd ../../..

echo "---> Linux Build <---"
# appimage build scatter/gathers it's dependencies from the host system -> use always-clean host.
docker build -t h900-builder ./tools/docker || exit 1
docker run --rm \
  --user "$(id -u):$(id -g)" \
  -e USER="$(whoami)" \
  -e HOME=/tmp \
  -e CCACHE_DISABLE=1 \
  -v "$(pwd):/src" \
  -w /src \
  h900-builder \
  ./tools/docker/build-linux.sh || exit 1

echo "---> MXE Build <---"
cd build/release/x86-64-windows
export PATH=/opt/mxe/usr/bin:$PATH
x86_64-w64-mingw32.shared-cmake -DCMAKE_BUILD_TYPE=Release ../../.. -DCMAKE_INSTALL_PREFIX=./install || exit 1
make package -j$(nproc) || exit 1
cd ../../..

echo "---> collecting build products <---"
mkdir out
cp build/release/docker/h900tools*.tar.gz out
cp build/release/docker/h900tools*.deb out
cp build/release/docker/H900*.AppImage out
cp build/release/x86-64-windows/h900tools*.exe out
cp build/release/x86-64-windows/h900tools*.zip out
ls -lh out
echo "---< done >---"
