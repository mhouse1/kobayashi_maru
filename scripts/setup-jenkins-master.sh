#!/bin/bash
# Quick Setup Script for Jenkins Master with Kobayashi Maru Tools
# Ubuntu Server 24.04.3 LTS

set -euo pipefail

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

log() { echo -e "${BLUE}[$(date +'%H:%M:%S')] $1${NC}"; }
success() { echo -e "${GREEN}✓ $1${NC}"; }

echo "======================================"
echo "Kobayashi Maru Jenkins Master Setup"
echo "======================================"
echo

# Update system
log "Updating system packages..."
sudo apt update && sudo apt upgrade -y

# Install essential tools
log "Installing development tools..."
sudo apt install -y \
    build-essential \
    cmake \
    git \
    curl \
    wget \
    unzip \
    python3 \
    python3-pip \
    openjdk-17-jdk \
    ca-certificates \
    gnupg

# Install Jenkins
log "Installing Jenkins..."
wget -q -O - https://pkg.jenkins.io/debian-stable/jenkins.io-2023.key | sudo apt-key add -
echo "deb https://pkg.jenkins.io/debian-stable binary/" | sudo tee /etc/apt/sources.list.d/jenkins.list
sudo apt update
sudo apt install -y jenkins

# Install ARM GCC Toolchain
log "Installing ARM GCC Toolchain..."
cd /tmp
wget -q "https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz"
sudo mkdir -p /opt/arm-gnu-toolchain
sudo tar -xf arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz -C /opt/arm-gnu-toolchain --strip-components=1
rm arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz

# Install Mono and Renode
log "Installing Renode simulation framework..."
sudo apt install -y gnupg ca-certificates
sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF
echo "deb https://download.mono-project.com/repo/ubuntu stable-focal main" | sudo tee /etc/apt/sources.list.d/mono-official-stable.list
sudo apt update
sudo apt install -y mono-complete

cd /tmp
wget -q "https://github.com/renode/renode/releases/download/v1.15.0/renode_1.15.0_amd64.deb"
sudo dpkg -i renode_1.15.0_amd64.deb || sudo apt-get install -f -y
rm renode_1.15.0_amd64.deb

# Install Python packages
log "Installing Python packages..."
python3 -m pip install --user numpy scipy matplotlib pyserial pytest

# Configure environment for Jenkins user
log "Configuring Jenkins environment..."
sudo tee -a /var/lib/jenkins/.bashrc > /dev/null <<'EOF'

# Kobayashi Maru Environment
export PATH="/opt/arm-gnu-toolchain/bin:$PATH"
export RENODE_PATH="/opt/renode"
EOF

sudo chown jenkins:jenkins /var/lib/jenkins/.bashrc

# Start Jenkins
log "Starting Jenkins..."
sudo systemctl start jenkins
sudo systemctl enable jenkins

# Wait for Jenkins to start
log "Waiting for Jenkins to initialize..."
timeout=60
while [ $timeout -gt 0 ] && ! sudo systemctl is-active --quiet jenkins; do
    sleep 2
    timeout=$((timeout-2))
done

success "Setup completed!"
echo
echo "==============================="
echo "SETUP SUMMARY"
echo "==============================="
echo "✓ Jenkins Master installed"
echo "✓ ARM GCC Toolchain available" 
echo "✓ Renode simulation ready"
echo "✓ Python environment configured"
echo
echo "JENKINS ACCESS:"
echo "  URL: http://localhost:8080"
echo "  Initial Password: $(sudo cat /var/lib/jenkins/secrets/initialAdminPassword 2>/dev/null || echo 'Check Jenkins logs')"
echo
echo "NEXT STEPS:"
echo "1. Open http://localhost:8080 in your browser"
echo "2. Use the initial admin password shown above"
echo "3. Install suggested plugins"
echo "4. Create your admin user"
echo "5. Create a new Pipeline job with your Jenkinsfile"
echo
echo "TEST COMMANDS:"
echo "  ARM GCC: /opt/arm-gnu-toolchain/bin/arm-none-eabi-gcc --version"
echo "  Renode: renode --version"
echo "  Jenkins: sudo systemctl status jenkins"