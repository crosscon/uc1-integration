FROM ubuntu:22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV TOOLCHAIN_URL=https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz
ENV TOOLCHAIN_DIR=/opt/arm-toolchain

# Install dependencies
RUN apt-get update && apt-get install -y \
    vim \
    git \
    wget \
    tar \
    xz-utils \
    build-essential \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Download and extract Arm GNU Toolchain
RUN mkdir -p $TOOLCHAIN_DIR && \
    wget -q $TOOLCHAIN_URL -O /tmp/toolchain.tar.xz && \
    tar -xf /tmp/toolchain.tar.xz -C $TOOLCHAIN_DIR --strip-components=1 && \
    rm /tmp/toolchain.tar.xz

# Add toolchain to PATH
ENV PATH="$TOOLCHAIN_DIR/bin:$PATH"

# Verify installation
RUN arm-none-eabi-gcc --version

WORKDIR /workspace

CMD ["/bin/bash"]
