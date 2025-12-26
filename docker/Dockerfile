# Dockerfile for Kobayashi Maru Jenkins Build Agent
# Based on Ubuntu 24.04.3 LTS

FROM ubuntu:24.04

LABEL maintainer="Kobayashi Maru Project"
LABEL description="Jenkins Build Agent for Kobayashi Maru Robot Project"
LABEL version="1.0"

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC
ENV PATH="/opt/arm-gnu-toolchain/bin:${PATH}"

# Update and install system packages
RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y \
        build-essential \
        cmake \
        ninja-build \
        git \
        curl \
        wget \
        unzip \
        python3 \
        python3-pip \
        python3-venv \
        pkg-config \
        libtool \
        autoconf \
        automake \
        gdb \
        telnet \
        screen \
        htop \
        vim \
        tree \
        ca-certificates \
        gnupg \
        software-properties-common \
        apt-transport-https \
        lsb-release \
        sudo \
        openssh-server \
        can-utils \
        libsocketcan-dev \
        openocd \
        minicom \
        picocom \
        dfu-util && \

# Zephyr dependencies
RUN apt-get update && apt-get install -y \
    python3-setuptools \
    python3-wheel \
    gperf \
    ccache \
    device-tree-compiler \
    make \
    gcc-multilib \
    g++-multilib \
    libsdl2-dev \
    udev \
    && rm -rf /var/lib/apt/lists/*

# Install west and Zephyr Python requirements
RUN python3 -m pip install --upgrade pip && \
    python3 -m pip install west && \
    west --version || true
# Optionally, Zephyr requirements.txt can be installed after west init/update
# RUN pip install -r zephyr/scripts/requirements.txt || true

# Install OpenJDK 17 for Jenkins
RUN apt-get update && \
    apt-get install -y openjdk-17-jdk && \
    rm -rf /var/lib/apt/lists/*

# Install Mono framework (required for Renode)
RUN gpg --homedir /tmp --no-default-keyring --keyring /usr/share/keyrings/mono-official-archive-keyring.gpg --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF && \
    echo "deb [signed-by=/usr/share/keyrings/mono-official-archive-keyring.gpg] https://download.mono-project.com/repo/ubuntu stable-focal main" | tee /etc/apt/sources.list.d/mono-official-stable.list && \
    apt-get update && \
    apt-get install -y mono-complete && \
    rm -rf /var/lib/apt/lists/*

# Install ARM GCC Toolchain
RUN mkdir -p /opt/arm-gnu-toolchain && \
    cd /tmp && \
    wget -O arm-toolchain.tar.xz "https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz" && \
    tar -xf arm-toolchain.tar.xz -C /opt/arm-gnu-toolchain --strip-components=1 && \
    rm arm-toolchain.tar.xz

# Install Renode
ENV RENODE_VERSION=1.15.0
RUN cd /tmp && \
    wget "https://github.com/renode/renode/releases/download/v${RENODE_VERSION}/renode_${RENODE_VERSION}_amd64.deb" && \
    dpkg -i "renode_${RENODE_VERSION}_amd64.deb" || apt-get install -f -y && \
    rm "renode_${RENODE_VERSION}_amd64.deb"

# Install Node.js
RUN curl -fsSL https://deb.nodesource.com/setup_lts.x | bash - && \
    apt-get install -y nodejs && \
    rm -rf /var/lib/apt/lists/*

# Install Python packages for simulation
RUN python3 -m pip install --upgrade pip && \
    python3 -m pip install \
        numpy \
        scipy \
        matplotlib \
        pyserial \
        pyyaml \
        pytest \
        flake8 \
        pylint

# Create jenkins user
RUN useradd -m -s /bin/bash jenkins && \
    echo "jenkins:jenkins" | chpasswd && \
    usermod -aG sudo jenkins && \
    echo "jenkins ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Create workspace directories
RUN mkdir -p /opt/jenkins/workspace && \
    chown -R jenkins:jenkins /opt/jenkins

# Create build script
COPY scripts/provision-jenkins-agent.sh /opt/jenkins/build-kobayashi-maru.sh
RUN chmod +x /opt/jenkins/build-kobayashi-maru.sh && \
    chown jenkins:jenkins /opt/jenkins/build-kobayashi-maru.sh

# Create environment setup script
RUN cat > /opt/jenkins/setup-env.sh << 'EOF'
#!/bin/bash
# Environment setup for Kobayashi Maru builds

export PATH="/opt/arm-gnu-toolchain/bin:${PATH}"

echo "Environment configured for Kobayashi Maru development"
echo "ARM GCC: $(which arm-none-eabi-gcc 2>/dev/null || echo 'Not found')"
echo "Renode: $(which renode 2>/dev/null || echo 'Not found')"
echo "Java: $(which java 2>/dev/null || echo 'Not found')"
EOF

RUN chmod +x /opt/jenkins/setup-env.sh && \
    chown jenkins:jenkins /opt/jenkins/setup-env.sh

# Set up SSH
RUN mkdir -p /var/run/sshd && \
    sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/' /etc/ssh/sshd_config && \
    sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin no/' /etc/ssh/sshd_config

# Create entrypoint script
RUN cat > /entrypoint.sh << 'EOF'
#!/bin/bash

# Start SSH daemon
service ssh start

# Source environment
source /opt/jenkins/setup-env.sh

# If Jenkins agent JAR is provided, run it
if [ -f "/home/jenkins/agent.jar" ]; then
    echo "Starting Jenkins agent..."
    cd /home/jenkins
    exec java -jar agent.jar "$@"
else
    echo "Jenkins agent JAR not found. Starting in interactive mode..."
    exec /bin/bash
fi
EOF

RUN chmod +x /entrypoint.sh

# Switch to jenkins user
USER jenkins
WORKDIR /home/jenkins

# Verify installations
RUN arm-none-eabi-gcc --version && \
    renode --version && \
    java -version && \
    python3 --version && \
    node --version

# Expose SSH port
EXPOSE 22

# Switch back to root for entrypoint
USER root

ENTRYPOINT ["/entrypoint.sh"]
CMD []