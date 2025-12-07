#!/bin/bash
# Docker-based Kobayashi Maru Development Setup

set -euo pipefail

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

log() { echo -e "${BLUE}[$(date +'%H:%M:%S')] $1${NC}"; }
success() { echo -e "${GREEN}✓ $1${NC}"; }
warning() { echo -e "${YELLOW}⚠ $1${NC}"; }

echo "=============================================="
echo "Kobayashi Maru - Docker Development Setup"
echo "=============================================="

# Check if Docker is available
if ! command -v docker &> /dev/null; then
    echo "❌ Docker not found. Please install Docker first."
    exit 1
fi

log "Docker version: $(docker --version)"

# Check if in project directory
if [[ ! -f "../firmware" ]] && [[ ! -d "simulation" ]]; then
    warning "Make sure you're in the scripts directory of the Kobayashi Maru project"
fi

cd "$(dirname "$0")/.."

# Build development environment
log "Building Kobayashi Maru development environment..."
echo "Trying simple ARM development environment first..."
docker compose -f docker/docker-compose.dev.yml build kobayashi-dev || {
    warning "Full build failed, trying simple ARM-only version..."
    docker build -f docker/Dockerfile.simple -t kobayashi-maru-dev .
}

success "Development environment built!"

echo
echo "========================================"
echo "USAGE OPTIONS"
echo "========================================"
echo

echo "🔨 FIRMWARE DEVELOPMENT:"
echo "  docker compose -f docker/docker-compose.dev.yml run --rm kobayashi-dev"
echo "  # Then in container:"
echo "  cd firmware && make"

echo
echo "🧪 SIMULATION TESTING:"
echo "  docker compose -f docker/docker-compose.dev.yml up renode-sim"
echo "  # Renode will be available on localhost:3456"

echo
echo "🔧 INTERACTIVE DEVELOPMENT:"
echo "  docker compose -f docker/docker-compose.dev.yml run --rm kobayashi-dev bash"

echo
echo "⚡ QUICK BUILD & TEST:"
echo "  docker compose -f docker/docker-compose.dev.yml run --rm kobayashi-dev bash -c '"
echo "    cd firmware && make && "
echo "    cd ../simulation/renode && "
echo "    timeout 30 renode --console --disable-xwt robot_simulation.resc"
echo "  '"

echo
echo "🏗️ JENKINS INTEGRATION:"
echo "  # Use jenkins-agent service for Jenkins builds"
echo "  docker compose -f docker/docker-compose.dev.yml up jenkins-agent"

echo
success "Setup complete! Choose an option above to get started."

# Test the environment
log "Testing development environment..."
docker compose -f docker/docker-compose.dev.yml run --rm kobayashi-dev arm-none-eabi-gcc --version || {
    log "Testing simple ARM environment..."
    docker run --rm -v "$(pwd)":/workspace kobayashi-maru-dev arm-none-eabi-gcc --version
}

success "ARM GCC toolchain is working in Docker!"

echo
echo "💡 TIP: All your source code is mounted into /workspace in the container"
echo "💡 TIP: Build artifacts persist in the build-cache volume"