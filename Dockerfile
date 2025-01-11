# Use Ubuntu 20.04 as the base image
FROM ubuntu:20.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update package list and install essential development packages
RUN apt-get update && \
    apt-get install -y \
        build-essential \  
        gdb \           
        net-tools \ 
        iputils-ping \      
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Default command to keep the container running in an interactive shell
CMD ["/bin/bash"]
