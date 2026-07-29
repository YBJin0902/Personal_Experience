FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Taipei

# 基本套件
RUN apt update && apt install -y --no-install-recommends \
    python3.8 \
    python3.8-dev \
    python3.8-venv \
    python3-pip \
    build-essential \
    cmake \
    make \
    git \
    wget \
    curl \
    unzip \
    vim \
    nano \
    file \
    ca-certificates \
    libglib2.0-0 \
    libgl1 \
    libsm6 \
    libxext6 \
    libxrender1 \
    libgomp1 \
    libusb-1.0-0 \
    && rm -rf /var/lib/apt/lists/*

# 指定 python / pip
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.8 1 && \
    update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.8 1

# 更新 pip
RUN python -m pip install --upgrade pip setuptools wheel

# 安裝 RKNN Toolkit2 2.3.2
RUN pip install --no-cache-dir \
    rknn-toolkit2==2.3.2

# 常用模型轉換輔助套件
RUN pip install --no-cache-dir \
    numpy \
    opencv-python-headless \
    pillow \
    onnx \
    onnxruntime \
    matplotlib

WORKDIR /work

CMD ["/bin/bash"]
