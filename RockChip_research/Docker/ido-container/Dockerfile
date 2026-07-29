FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# 加入 arm64 套件來源
RUN dpkg --add-architecture arm64 && \
    sed -i 's|deb http://archive.ubuntu.com/ubuntu/|deb [arch=amd64] http://archive.ubuntu.com/ubuntu/|g' /etc/apt/sources.list && \
    sed -i 's|deb http://security.ubuntu.com/ubuntu/|deb [arch=amd64] http://security.ubuntu.com/ubuntu/|g' /etc/apt/sources.list && \
    echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports jammy main restricted universe multiverse" > /etc/apt/sources.list.d/arm64.list && \
    echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports jammy-updates main restricted universe multiverse" >> /etc/apt/sources.list.d/arm64.list && \
    echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports jammy-security main restricted universe multiverse" >> /etc/apt/sources.list.d/arm64.list

# 基本編譯工具 + aarch64 交叉編譯器
RUN apt update && apt install -y --no-install-recommends \
    build-essential \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    cmake \
    make \
    ninja-build \
    pkg-config \
    git \
    vim \
    nano \
    wget \
    curl \
    unzip \
    file \
    rsync \
    ssh \
    gdb-multiarch \
    qemu-user-static \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# RK3588 常用 arm64 開發函式庫
RUN apt update && apt install -y --no-install-recommends \
    libc6-dev:arm64 \
    libdrm-dev:arm64 \
    libudev-dev:arm64 \
    libusb-1.0-0-dev:arm64 \
    libi2c-dev:arm64 \
    zlib1g-dev:arm64 \
    && rm -rf /var/lib/apt/lists/*

# 如果你的程式有用 GTK GUI，再取消下面這段註解
# RUN apt update && apt install -y --no-install-recommends \
#     libgtk-3-dev:arm64 \
#     && rm -rf /var/lib/apt/lists/*

# 如果你的程式有用 OpenCV，再取消下面這段註解
# RUN apt update && apt install -y --no-install-recommends \
#     libopencv-dev:arm64 \
#     && rm -rf /var/lib/apt/lists/*

ENV CROSS_COMPILE=aarch64-linux-gnu-
ENV CC=aarch64-linux-gnu-gcc
ENV CXX=aarch64-linux-gnu-g++
ENV AR=aarch64-linux-gnu-ar
ENV STRIP=aarch64-linux-gnu-strip

# pkg-config 指向 arm64 函式庫，避免抓到 x86_64 的 lib
ENV PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig
ENV PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig

WORKDIR /work

CMD ["/bin/bash"]
