#!/bin/bash
# Simple Kobayashi Maru Tools Installation
# For existing Jenkins on Ubuntu 24.04

set -euo pipefail

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

log() { echo -e "${BLUE}[$(date +'%H:%M:%S')] $1${NC}"; }
success() { echo -e "${GREEN}✓ $1${NC}"; }
warning() { echo -e "${YELLOW}⚠ $1${NC}"; }

echo "=============================================="
echo "Installing Essential Kobayashi Maru Tools"
echo "=============================================="

# Install build tools
log "Installing build tools..."
sudo apt update --fix-missing || {
    warning "Package update had issues, continuing anyway..."
}
sudo apt install -y build-essential cmake git python3 python3-pip wget

# Install ARM GCC Toolchain
log "Installing ARM GCC Toolchain..."
if [[ ! -d "/opt/arm-gnu-toolchain" ]]; then
    cd /tmp
    wget -q "https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz"
    sudo mkdir -p /opt/arm-gnu-toolchain
    sudo tar -xf arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz -C /opt/arm-gnu-toolchain --strip-components=1
    rm arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz
    success "ARM GCC Toolchain installed"
else
    success "ARM GCC Toolchain already present"
fi

# Install Mono (using Ubuntu packages instead of external repo)
log "Installing Mono from Ubuntu repositories..."
sudo apt install -y mono-complete || {
    warning "Mono installation failed, trying alternative method..."
    sudo apt install -y mono-runtime mono-devel || true
}

# Install Renode
log "Installing Renode..."
if ! command -v renode &> /dev/null; then
    cd /tmp
    wget -q "https://github.com/renode/renode/releases/download/v1.15.0/renode_1.15.0_amd64.deb"
    sudo dpkg -i renode_1.15.0_amd64.deb || sudo apt-get install -f -y
    rm renode_1.15.0_amd64.deb
    success "Renode installed"
else
    success "Renode already installed"
fi

# Install Python packages
log "Installing Python packages..."
python3 -m pip install --user numpy scipy matplotlib pyserial pytest

# Add ARM toolchain to PATH
log "Configuring PATH..."
if ! grep -q "/opt/arm-gnu-toolchain/bin" /etc/environment 2>/dev/null; then
    echo 'PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/opt/arm-gnu-toolchain/bin"' | sudo tee /etc/environment > /dev/null
    success "ARM toolchain added to system PATH"
fi

# Configure Jenkins user
if id "jenkins" &>/dev/null; then
    log "Configuring Jenkins user..."
    sudo -u jenkins tee -a /var/lib/jenkins/.bashrc > /dev/null <<'EOF' || true

# Kobayashi Maru Tools
export PATH="/opt/arm-gnu-toolchain/bin:$PATH"
EOF
    sudo -u jenkins python3 -m pip install --user numpy scipy matplotlib pyserial pytest 2>/dev/null || true
    success "Jenkins user configured"
fi

# Create verification script
log "Creating verification script..."
sudo tee /usr/local/bin/test-kobayashi-tools > /dev/null <<'EOF'
#!/bin/bash
echo "Testing Kobayashi Maru Tools..."
echo

echo "ARM GCC:"
if /opt/arm-gnu-toolchain/bin/arm-none-eabi-gcc --version &>/dev/null; then
    /opt/arm-gnu-toolchain/bin/arm-none-eabi-gcc --version | head -1
else
    echo "  ❌ Not found"
fi

echo
echo "Renode:"
if command -v renode &> /dev/null; then
    echo "  ✓ Renode available"
else
    echo "  ❌ Not found"
fi

echo
echo "Python:"
python3 -c "
try:
    import numpy; print('  ✓ NumPy:', numpy.__version__)
except ImportError:
    print('  ❌ NumPy not available')
"

echo
echo "Build tools:"
echo "  GCC: $(gcc --version | head -1)"
echo "  Git: $(git --version)"
echo "  Docker: $(docker --version 2>/dev/null)"
EOF

sudo chmod +x /usr/local/bin/test-kobayashi-tools

echo
success "Installation completed!"
echo
echo "VERIFICATION:"
echo "  Run: test-kobayashi-tools"
echo "  Or: /opt/arm-gnu-toolchain/bin/arm-none-eabi-gcc --version"
echo
echo "JENKINS SETUP:"
echo "  1. Access Jenkins at http://192.168.50.65:8080"
echo "  2. Create Pipeline job with the Jenkinsfile"
echo "  3. Restart Jenkins: sudo systemctl restart jenkins"
echo
warning "Reboot recommended: sudo reboot"