FROM debian:13

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build git ca-certificates \
    autoconf automake libtool pkg-config \
    libhidapi-dev libzip-dev libcurl4-openssl-dev \
    qt6-base-dev qt6-base-private-dev qt6-tools-dev qt6-tools-dev-tools qt6-svg-dev \
    libboost-dev libxkbcommon-dev \
    wget imagemagick file \
    nsis \
    && rm -rf /var/lib/apt/lists/*

ENV QT_QPA_PLATFORM=offscreen

WORKDIR /src
