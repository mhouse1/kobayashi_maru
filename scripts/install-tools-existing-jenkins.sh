#!/bin/bash
# Add Kobayashi Maru Tools to Existing Jenkins Server
# For Jenkins at http://192.168.50.65:8080

set -euo pipefail

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

log() { echo -e "${BLUE}[$(date +'%H:%M:%S')] $1${NC}"; }
success() { echo -e "${GREEN}✓ $1${NC}"; }
warning() { echo -e "${YELLOW}⚠ $1${NC}"; }

JENKINS_URL="http://192.168.50.65:8080"

echo "================================================"
echo "Installing Kobayashi Maru Tools for Jenkins"
echo "Jenkins URL: $JENKINS_URL"
echo "================================================"
echo

# Check if this is the Jenkins server
if ! command -v jenkins &> /dev/null && ! systemctl is-active --quiet jenkins 2>/dev/null; then
    warning "This doesn't appear to be the Jenkins server"
    warning "Make sure you're running this on 192.168.50.65"
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Update system
log "Updating system packages..."
sudo apt update

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

# Install build tools
log "Installing build tools..."
sudo apt install -y build-essential cmake git python3 python3-pip

# Install Mono and Renode
log "Installing Renode simulation framework..."
if ! command -v renode &> /dev/null; then
    # Install Mono first
    if ! command -v mono &> /dev/null; then
        sudo apt install -y gnupg ca-certificates
        wget -qO- https://keys.openpgp.org/vks/v1/by-fingerprint/3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF | sudo gpg --dearmor -o /usr/share/keyrings/mono-official-archive-keyring.gpg
        echo "deb [signed-by=/usr/share/keyrings/mono-official-archive-keyring.gpg] https://download.mono-project.com/repo/ubuntu stable-focal main" | sudo tee /etc/apt/sources.list.d/mono-official-stable.list
        sudo apt update
        sudo apt install -y mono-complete
    fi
    
    # Install Renode
    cd /tmp
    wget -q "https://github.com/renode/renode/releases/download/v1.15.0/renode_1.15.0_amd64.deb"
    sudo dpkg -i renode_1.15.0_amd64.deb || sudo apt-get install -f -y
    rm renode_1.15.0_amd64.deb
    success "Renode installed"
else
    success "Renode already installed"
fi

# Install Python packages for simulation
log "Installing Python simulation packages..."
python3 -m pip install --user numpy scipy matplotlib pyserial pytest

# Add tools to system PATH
log "Adding tools to system PATH..."
if ! grep -q "/opt/arm-gnu-toolchain/bin" /etc/environment; then
    echo 'PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/opt/arm-gnu-toolchain/bin"' | sudo tee /etc/environment > /dev/null
    success "ARM toolchain added to system PATH"
fi

# Configure for Jenkins user if it exists
if id "jenkins" &>/dev/null; then
    log "Configuring environment for Jenkins user..."
    
    # Add to Jenkins user's bashrc
    if [[ ! -f "/var/lib/jenkins/.bashrc" ]] || ! grep -q "arm-gnu-toolchain" /var/lib/jenkins/.bashrc 2>/dev/null; then
        sudo -u jenkins tee -a /var/lib/jenkins/.bashrc > /dev/null <<'EOF'

# Kobayashi Maru Development Tools
export PATH="/opt/arm-gnu-toolchain/bin:$PATH"
export RENODE_ROOT="/opt/renode"
EOF
        sudo chown jenkins:jenkins /var/lib/jenkins/.bashrc
        success "Jenkins user environment configured"
    fi
    
    # Install Python packages for jenkins user
    sudo -u jenkins python3 -m pip install --user numpy scipy matplotlib pyserial pytest 2>/dev/null || true
else
    warning "Jenkins user not found - tools installed system-wide"
fi

# Create test script
log "Creating verification script..."
sudo tee /usr/local/bin/test-kobayashi-tools > /dev/null <<'EOF'
#!/bin/bash
echo "Testing Kobayashi Maru Development Tools..."
echo

echo "ARM GCC Toolchain:"
if command -v arm-none-eabi-gcc &> /dev/null; then
    arm-none-eabi-gcc --version | head -1
else
    echo "  ❌ Not found in PATH"
fi

echo
echo "Renode Simulation:"
if command -v renode &> /dev/null; then
    renode --version 2>/dev/null || echo "  ✓ Renode installed"
else
    echo "  ❌ Not found"
fi

echo
echo "Python Packages:"
python3 -c "
try:
    import numpy; print('  ✓ NumPy:', numpy.__version__)
except ImportError:
    print('  ❌ NumPy not available')

try:
    import serial; print('  ✓ PySerial available')
except ImportError:
    print('  ❌ PySerial not available')
"

echo
echo "Build Tools:"
echo "  GCC: $(gcc --version | head -1)"
echo "  CMake: $(cmake --version | head -1)"
echo "  Git: $(git --version)"
EOF

sudo chmod +x /usr/local/bin/test-kobayashi-tools

echo
success "Installation completed!"
echo
echo "==============================================="
echo "INSTALLATION SUMMARY"
echo "==============================================="
echo "✓ ARM GCC Toolchain (/opt/arm-gnu-toolchain)"
echo "✓ Renode simulation framework"
echo "✓ Build tools (GCC, CMake, Git)"
echo "✓ Python simulation packages"
echo "✓ Environment configured"
echo
echo "VERIFICATION:"
echo "  Run: test-kobayashi-tools"
echo
echo "JENKINS PIPELINE:"
echo "  1. Use the provided Jenkinsfile in your repository"
echo "  2. Create a Pipeline job in Jenkins at:"
echo "     $JENKINS_URL"
echo "  3. Point it to your Git repository"
echo "  4. The pipeline will run on the master node"
echo
echo "RENODE TESTING:"
echo "  cd simulation/renode"
echo "  renode robot_simulation.resc"
echo
warning "Reboot recommended to ensure PATH changes take effect"
echo "  sudo reboot"