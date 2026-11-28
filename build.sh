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
docker build -t remote-workbench-builder ./tools/docker || exit 1
docker run --rm \
  --user "$(id -u):$(id -g)" \
  -e USER="$(whoami)" \
  -e HOME=/tmp \
  -e CCACHE_DISABLE=1 \
  -v "$(pwd):/src" \
  -w /src \
  remote-workbench-builder \
  ./tools/docker/build-linux.sh || exit 1

echo "---> MXE Build <---"
cd build/release/x86-64-windows
export PATH=/opt/mxe/usr/bin:$PATH
x86_64-w64-mingw32.shared-cmake -DCMAKE_BUILD_TYPE=Release ../../.. -DCMAKE_INSTALL_PREFIX=./install || exit 1
make package -j$(nproc) || exit 1
cd ../../..

echo "---> collecting build products <---"
mkdir out
cp build/release/docker/remoteWorkbench*.tar.gz out
cp build/release/docker/remoteworkbench*.deb out
cp build/release/docker/RemoteWorkbench*.AppImage out
cp build/release/x86-64-windows/remoteWorkbench*.exe out
cp build/release/x86-64-windows/remoteWorkbench*.zip out
ls -lh out
echo "---< done >---"
