# Use Ubuntu 20.04 as the base image
FROM ubuntu:20.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update package list and install essential packages
RUN apt-get update && \
    apt-get install -y \
        ca-certificates \
        curl \
        vim \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy project files into the container (optional)
# COPY . /app

# Default command
CMD ["/bin/bash"]
