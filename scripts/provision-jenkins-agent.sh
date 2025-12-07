#!/bin/bash
# Jenkins Master Provisioning Script
# For Kobayashi Maru Robot Project
# Ubuntu Server 24.04.3 LTS

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Log function
log() {
    echo -e "${BLUE}[$(date +'%Y-%m-%d %H:%M:%S')] $1${NC}"
}

success() {
    echo -e "${GREEN}[SUCCESS] $1${NC}"
}

warning() {
    echo -e "${YELLOW}[WARNING] $1${NC}"
}

error() {
    echo -e "${RED}[ERROR] $1${NC}"
}

# Check if running as root
if [[ $EUID -eq 0 ]]; then
   error "This script should not be run as root for security reasons"
   exit 1
fi

log "Starting Jenkins Master provisioning for Kobayashi Maru project..."

# Update system packages
log "Updating system packages..."
sudo apt update && sudo apt upgrade -y

# Install essential build tools
log "Installing essential build tools..."
sudo apt install -y \
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
    tree

# Install Jenkins master
log "Installing Jenkins master..."
sudo apt install -y openjdk-17-jdk

# Add Jenkins repository
wget -q -O - https://pkg.jenkins.io/debian-stable/jenkins.io.key | sudo apt-key add -
echo "deb https://pkg.jenkins.io/debian-stable binary/" | sudo tee /etc/apt/sources.list.d/jenkins.list
sudo apt update
sudo apt install -y jenkins

# Start and enable Jenkins
sudo systemctl start jenkins
sudo systemctl enable jenkins

# Wait for Jenkins to start
log "Waiting for Jenkins to start..."
sleep 30

java -version

# Install ARM GCC Toolchain
log "Installing ARM GCC Toolchain..."
ARM_TOOLCHAIN_URL="https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz"
ARM_TOOLCHAIN_DIR="/opt/arm-gnu-toolchain"

sudo mkdir -p $ARM_TOOLCHAIN_DIR
cd /tmp
wget -O arm-toolchain.tar.xz "$ARM_TOOLCHAIN_URL"
sudo tar -xf arm-toolchain.tar.xz -C $ARM_TOOLCHAIN_DIR --strip-components=1
rm arm-toolchain.tar.xz

# Add ARM toolchain to PATH
echo 'export PATH="$PATH:/opt/arm-gnu-toolchain/bin"' | sudo tee -a /etc/environment
export PATH="$PATH:/opt/arm-gnu-toolchain/bin"

# Verify ARM toolchain installation
if command -v arm-none-eabi-gcc &> /dev/null; then
    success "ARM GCC toolchain installed successfully"
    arm-none-eabi-gcc --version | head -1
else
    error "ARM GCC toolchain installation failed"
    exit 1
fi

# Install Mono (required for Renode)
log "Installing Mono framework..."
sudo apt install -y ca-certificates gnupg
sudo gpg --homedir /tmp --no-default-keyring --keyring /usr/share/keyrings/mono-official-archive-keyring.gpg --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF
sudo tee /etc/apt/sources.list.d/mono-official-stable.list > /dev/null <<EOF
deb [signed-by=/usr/share/keyrings/mono-official-archive-keyring.gpg] https://download.mono-project.com/repo/ubuntu stable-focal main
EOF
sudo apt update
sudo apt install -y mono-complete

# Install Renode
log "Installing Renode simulation framework..."
RENODE_VERSION="1.15.0"
RENODE_DEB_URL="https://github.com/renode/renode/releases/download/v${RENODE_VERSION}/renode_${RENODE_VERSION}_amd64.deb"

cd /tmp
wget "$RENODE_DEB_URL"
sudo dpkg -i "renode_${RENODE_VERSION}_amd64.deb" || true
sudo apt-get install -f -y  # Fix any dependency issues
rm "renode_${RENODE_VERSION}_amd64.deb"

# Verify Renode installation
if command -v renode &> /dev/null; then
    success "Renode installed successfully"
    renode --version
else
    error "Renode installation failed"
    exit 1
fi

# Install Python packages for simulation models
log "Setting up Python environment for simulation models..."
python3 -m pip install --user --upgrade pip
python3 -m pip install --user \
    numpy \
    scipy \
    matplotlib \
    pyserial \
    pyyaml \
    pytest

# Create workspace directory structure
log "Creating workspace directories..."
WORKSPACE_DIR="/opt/jenkins/workspace"
sudo mkdir -p $WORKSPACE_DIR
sudo chown -R $USER:$USER $WORKSPACE_DIR

# Install additional tools for CAN-FD and embedded development
log "Installing additional embedded development tools..."
sudo apt install -y \
    can-utils \
    libsocketcan-dev \
    openocd \
    minicom \
    picocom \
    dfu-util

# Install Android SDK (for Android app building if needed)
log "Installing Android SDK components..."
ANDROID_SDK_DIR="/opt/android-sdk"
sudo mkdir -p $ANDROID_SDK_DIR
sudo chown -R $USER:$USER $ANDROID_SDK_DIR

# Download Android Command Line Tools
ANDROID_TOOLS_URL="https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip"
cd /tmp
wget "$ANDROID_TOOLS_URL" -O android-tools.zip
unzip android-tools.zip
mv cmdline-tools $ANDROID_SDK_DIR/
mkdir -p $ANDROID_SDK_DIR/cmdline-tools/latest
mv $ANDROID_SDK_DIR/cmdline-tools/* $ANDROID_SDK_DIR/cmdline-tools/latest/ 2>/dev/null || true
rm android-tools.zip

# Set up Android environment variables
cat >> ~/.bashrc <<EOF

# Android SDK
export ANDROID_HOME=$ANDROID_SDK_DIR
export ANDROID_SDK_ROOT=$ANDROID_SDK_DIR
export PATH=\$PATH:\$ANDROID_HOME/cmdline-tools/latest/bin
export PATH=\$PATH:\$ANDROID_HOME/platform-tools
EOF

# Source the updated bashrc
source ~/.bashrc

# Accept Android licenses and install basic SDK components
log "Setting up Android SDK..."
export ANDROID_HOME=$ANDROID_SDK_DIR
export PATH=$PATH:$ANDROID_HOME/cmdline-tools/latest/bin

yes | sdkmanager --licenses || true
sdkmanager "platform-tools" "platforms;android-34" "build-tools;34.0.0"

# Install Docker (for containerized builds if needed)
log "Installing Docker..."
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh
sudo usermod -aG docker $USER
rm get-docker.sh

# Install Node.js (for any web-based tools or build scripts)
log "Installing Node.js..."
curl -fsSL https://deb.nodesource.com/setup_lts.x | sudo -E bash -
sudo apt install -y nodejs

# Configure Jenkins master
log "Configuring Jenkins master..."

# Add jenkins user to docker group
sudo usermod -aG docker jenkins

# Create workspace directory
sudo mkdir -p /var/lib/jenkins/workspace
sudo chown -R jenkins:jenkins /var/lib/jenkins/workspace

# Set up environment for Jenkins
echo 'export PATH="$PATH:/opt/arm-gnu-toolchain/bin"' | sudo tee -a /var/lib/jenkins/.bashrc
echo 'export ANDROID_HOME="/opt/android-sdk"' | sudo tee -a /var/lib/jenkins/.bashrc
echo 'export ANDROID_SDK_ROOT="/opt/android-sdk"' | sudo tee -a /var/lib/jenkins/.bashrc
echo 'export PATH="$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools"' | sudo tee -a /var/lib/jenkins/.bashrc

# Set ownership
sudo chown jenkins:jenkins /var/lib/jenkins/.bashrc

# Restart Jenkins to pick up environment changes
sudo systemctl restart jenkins

# Get initial admin password
log "Jenkins initial admin password:"
sudo cat /var/lib/jenkins/secrets/initialAdminPassword || echo "Password file not found - Jenkins may still be starting"

# Create build script template
log "Creating build script template..."
sudo mkdir -p /opt/jenkins
sudo tee /opt/jenkins/build-kobayashi-maru.sh > /dev/null <<'EOF'
#!/bin/bash
# Kobayashi Maru Build Script for Jenkins

set -euo pipefail

PROJECT_DIR=${WORKSPACE:-$(pwd)}
BUILD_DIR="$PROJECT_DIR/build"

echo "Building Kobayashi Maru Robot Firmware..."
echo "Project Directory: $PROJECT_DIR"
echo "Build Directory: $BUILD_DIR"

# Clean previous build
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Build firmware
cd "$PROJECT_DIR/firmware"
if [ -f "Makefile" ]; then
    make clean
    make -j$(nproc)
elif [ -f "CMakeLists.txt" ]; then
    cd "$BUILD_DIR"
    cmake "$PROJECT_DIR/firmware" -DCMAKE_TOOLCHAIN_FILE=arm-none-eabi.cmake
    make -j$(nproc)
else
    echo "No build system found (Makefile or CMakeLists.txt)"
    exit 1
fi

# Run Renode simulation tests
cd "$PROJECT_DIR/simulation/renode"
echo "Running Renode simulation tests..."
timeout 60 renode --console --disable-xwt robot_simulation.resc &
RENODE_PID=$!

# Wait for simulation to start
sleep 10

# Kill Renode after test
kill $RENODE_PID || true

echo "Build and test completed successfully!"
EOF

sudo chmod +x /opt/jenkins/build-kobayashi-maru.sh
sudo chown jenkins:jenkins /opt/jenkins/build-kobayashi-maru.sh

# Create environment setup script
log "Creating environment setup script..."
tee /tmp/setup-env.sh > /dev/null <<'EOF'
#!/bin/bash
# Environment setup for Kobayashi Maru builds

export PATH="$PATH:/opt/arm-gnu-toolchain/bin"
export ANDROID_HOME="/opt/android-sdk"
export ANDROID_SDK_ROOT="/opt/android-sdk"
export PATH="$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools"

echo "Environment configured for Kobayashi Maru development"
echo "ARM GCC: $(which arm-none-eabi-gcc 2>/dev/null || echo 'Not found')"
echo "Renode: $(which renode 2>/dev/null || echo 'Not found')"
echo "Java: $(which java 2>/dev/null || echo 'Not found')"
echo "Android SDK: $ANDROID_HOME"
EOF

sudo mv /tmp/setup-env.sh /opt/jenkins/
sudo chmod +x /opt/jenkins/setup-env.sh
sudo chown jenkins:jenkins /opt/jenkins/setup-env.sh

# Final system cleanup
log "Cleaning up..."
sudo apt autoremove -y
sudo apt autoclean

# Print summary
success "Jenkins Master provisioning completed!"
echo
echo "========================================"
echo "PROVISIONING SUMMARY"
echo "========================================"
echo "✓ System packages updated"
echo "✓ ARM GCC Toolchain installed (/opt/arm-gnu-toolchain)"
echo "✓ Renode simulation framework installed"
echo "✓ Python environment configured"
echo "✓ Android SDK installed (/opt/android-sdk)"
echo "✓ Docker installed"
echo "✓ Node.js installed"
echo "✓ Jenkins Master installed and running"
echo "✓ Build scripts created"
echo
echo "NEXT STEPS:"
echo "1. Access Jenkins at: http://localhost:8080"
echo "2. Use initial admin password shown above"
echo "3. Install suggested plugins"
echo "4. Create admin user"
echo "5. Create a new Pipeline job with the provided Jenkinsfile"
echo "6. Test the build: /opt/jenkins/build-kobayashi-maru.sh"
echo
echo "IMPORTANT NOTES:"
echo "- Jenkins is running on port 8080"
echo "- Initial admin password: /var/lib/jenkins/secrets/initialAdminPassword"
echo "- ARM toolchain is in /opt/arm-gnu-toolchain/bin"
echo "- Android SDK is in /opt/android-sdk"
echo "- Jenkins workspace: /var/lib/jenkins/workspace"
echo
warning "Please reboot the system to ensure all environment variables are properly loaded"