FROM debian:11

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Taipei

RUN apt-get update && \
    apt-get install -y \
        build-essential \
        gcc \
        g++ \
        make \
        cmake \
        gdb \
        git \
        vim \
        nano \
        wget \
        curl \
        pkg-config \
        python3 \
        python3-pip \
        file \
        sudo \
        locales \
        tzdata \
        libgtk-3-dev \
        libdrm-dev \
        libusb-1.0-0-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN sed -i 's/^# *\(en_US.UTF-8 UTF-8\)/\1/' /etc/locale.gen && locale-gen

ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

WORKDIR /workspace

CMD ["/bin/bash"]
