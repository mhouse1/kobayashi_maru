#!/bin/bash
# Deployment script for Kobayashi Maru Jenkins Master
# Ubuntu Server 24.04.3 LTS

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log() { echo -e "${BLUE}[$(date +'%Y-%m-%d %H:%M:%S')] $1${NC}"; }
success() { echo -e "${GREEN}[SUCCESS] $1${NC}"; }
warning() { echo -e "${YELLOW}[WARNING] $1${NC}"; }
error() { echo -e "${RED}[ERROR] $1${NC}"; }

# Configuration
DEPLOYMENT_TYPE=${1:-"native"}  # native or docker
JENKINS_URL=${JENKINS_URL:-"http://192.168.50.65:8080"}
JENKINS_PORT=${JENKINS_PORT:-"8080"}

usage() {
    cat << EOF
Usage: $0 [deployment_type]

Deployment Types:
  native    - Install tools on existing Jenkins server (default)
  docker    - Deploy using Docker containers

Environment Variables:
  JENKINS_URL         - Existing Jenkins master URL (default: http://192.168.50.65:8080)
  JENKINS_PORT        - Jenkins master port (default: 8080)

Examples:
  # Install tools on existing Jenkins server
  $0 native

  # Use different Jenkins URL
  JENKINS_URL=http://your-jenkins:8080 $0 native

  # Interactive setup
  $0
EOF
}

check_prerequisites() {
    log "Checking prerequisites..."
    
    # Check OS
    if ! grep -q "Ubuntu 24.04" /etc/os-release; then
        warning "This script is designed for Ubuntu Server 24.04.3 LTS"
        read -p "Continue anyway? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
    
    # Check if running as root
    if [[ $EUID -eq 0 ]]; then
        error "This script should not be run as root for security reasons"
        exit 1
    fi
    
    # Check sudo access
    if ! sudo -n true 2>/dev/null; then
        error "This script requires sudo access"
        exit 1
    fi
    
    success "Prerequisites check passed"
}

deploy_native() {
    log "Installing Kobayashi Maru tools on existing Jenkins master..."
    
    # Check if Jenkins is running
    if ! systemctl is-active --quiet jenkins 2>/dev/null; then
        warning "Jenkins service not found or not running locally"
        warning "This script will install tools system-wide for Jenkins to use"
    fi
    
    # Check if provisioning script exists
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    PROVISION_SCRIPT="$SCRIPT_DIR/provision-jenkins-agent.sh"
    
    if [[ ! -f "$PROVISION_SCRIPT" ]]; then
        error "Provisioning script not found: $PROVISION_SCRIPT"
        exit 1
    fi
    
    log "Running provisioning script for development tools..."
    chmod +x "$PROVISION_SCRIPT"
    "$PROVISION_SCRIPT"
    
    success "Tools installation completed"
    
    echo
    echo "Jenkins Master Information:"
    echo "  URL: ${JENKINS_URL}"
    if systemctl is-active --quiet jenkins 2>/dev/null; then
        echo "  Local Status: $(sudo systemctl is-active jenkins)"
        echo "  Local Initial Password: $(sudo cat /var/lib/jenkins/secrets/initialAdminPassword 2>/dev/null || echo 'N/A - Jenkins already configured')"
    else
        echo "  Status: External Jenkins server (not managed locally)"
    fi
}

deploy_docker() {
    log "Starting Docker deployment for Jenkins master..."
    
    # Check if Docker is installed
    if ! command -v docker &> /dev/null; then
        log "Installing Docker..."
        curl -fsSL https://get.docker.com -o get-docker.sh
        sudo sh get-docker.sh
        sudo usermod -aG docker $USER
        rm get-docker.sh
        
        warning "Docker installed. You may need to log out and back in for group changes to take effect"
        warning "Or run: newgrp docker"
    fi
    
    # Check if Docker Compose is available
    if ! command -v docker-compose &> /dev/null && ! docker compose version &> /dev/null; then
        log "Installing Docker Compose..."
        sudo apt update
        sudo apt install -y docker-compose-plugin
    fi
    
    # Navigate to docker directory
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    DOCKER_DIR="$SCRIPT_DIR/../docker"
    
    if [[ ! -d "$DOCKER_DIR" ]]; then
        error "Docker directory not found: $DOCKER_DIR"
        exit 1
    fi
    
    cd "$DOCKER_DIR"
    
    # Create environment file
    cat > .env << EOF
JENKINS_URL=${JENKINS_URL}
JENKINS_PORT=${JENKINS_PORT}
EOF
    
    log "Building Docker image..."
    docker-compose build jenkins-agent
    
    log "Starting Jenkins master container..."
    docker-compose up -d jenkins-master
    
    success "Docker deployment completed"
    
    echo
    echo "Container Status:"
    docker-compose ps
    
    echo
    echo "To view logs: docker-compose logs -f jenkins-master"
    echo "To enter container: docker-compose exec jenkins-master bash"
}



interactive_setup() {
    echo
    echo "========================================"
    echo "Kobayashi Maru Tools for Jenkins Master"
    echo "========================================"
    echo
    
    # Get deployment type
    echo "Select deployment type:"
    echo "1) Install tools on existing Jenkins server (native)"
    echo "2) Docker deployment"
    read -p "Choice (1-2): " -n 1 -r
    echo
    
    case $REPLY in
        1) DEPLOYMENT_TYPE="native" ;;
        2) DEPLOYMENT_TYPE="docker" ;;
        *) error "Invalid choice"; exit 1 ;;
    esac
    
    # Get Jenkins configuration
    read -p "Jenkins URL [${JENKINS_URL}]: " input_url
    JENKINS_URL=${input_url:-$JENKINS_URL}
    
    echo
    echo "Configuration Summary:"
    echo "  Deployment Type: $DEPLOYMENT_TYPE"
    echo "  Jenkins URL: $JENKINS_URL"
    echo
    
    read -p "Proceed with deployment? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Deployment cancelled"
        exit 0
    fi
}

# Main execution
main() {
    log "Installing Kobayashi Maru Development Tools"
    
    # Show usage if requested
    if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
        usage
        exit 0
    fi
    
    # Run interactive setup if no parameters provided
    if [[ $# -eq 0 ]]; then
        interactive_setup
    fi
    
    # Validate deployment type
    if [[ "$DEPLOYMENT_TYPE" != "native" && "$DEPLOYMENT_TYPE" != "docker" ]]; then
        error "Invalid deployment type: $DEPLOYMENT_TYPE"
        usage
        exit 1
    fi
    
    check_prerequisites
    
    case "$DEPLOYMENT_TYPE" in
        native)
            deploy_native
            ;;
        docker)
            deploy_docker
            ;;
    esac
    
    echo
    success "Installation completed successfully! 🎉"
    echo
    echo "Next Steps:"
    echo "1. Access your Jenkins at: ${JENKINS_URL}"
    echo "2. Copy the Jenkinsfile to your repository"
    echo "3. Create a new Pipeline job pointing to your repository"
    echo "4. Configure the pipeline to use label 'master' or no specific label"
    echo "5. Test the build pipeline"
    echo
    echo "Build Testing:"
    echo "  Manual test: /opt/jenkins/build-kobayashi-maru.sh"
    echo "  Pipeline: Create job with provided Jenkinsfile"
    echo
    echo "Verification Commands:"
    echo "  ARM GCC: arm-none-eabi-gcc --version"
    echo "  Renode: renode --version"
    echo "  Python: python3 -c 'import numpy; print(\"NumPy available\")'  "
    echo
    warning "Remember to reboot if using native installation to ensure all environment variables are loaded"
}

# Execute main function with all arguments
main "$@"