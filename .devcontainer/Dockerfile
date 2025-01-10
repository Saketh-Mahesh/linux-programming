FROM ubuntu:latest
# Install essential tools
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    gcc \
    g++ \
    gdb \
    cmake \
    make \
    ninja-build \
    git \
    curl \
    wget \
    iputils-ping \
    net-tools \
    netcat \
    tcpdump \
    strace \
    ltrace \
    valgrind \
    man-db \
    pkg-config \
    python3 \
    python3-pip \
    clang \
    clangd \
    clang-format \
    libssl-dev \
    zlib1g-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /workspace
