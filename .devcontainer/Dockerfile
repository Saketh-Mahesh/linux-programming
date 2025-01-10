FROM ubuntu:latest

# Install essential tools
RUN apt-get update && apt-get install -y \
    build-essential \
    gdb \
    cmake \
    valgrind \
    address-sanitizer \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /workspace
