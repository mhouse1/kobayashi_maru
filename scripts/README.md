# Jenkins Master Tool Setup for Kobayashi Maru

This directory contains scripts and configurations to set up development tools on an existing Jenkins master for the Kobayashi Maru robot project.

## Quick Start

### Option 1: Simple Tool Installation (Recommended)

```bash
# Run on your Jenkins server (192.168.50.65)
chmod +x scripts/install-tools-existing-jenkins.sh
./scripts/install-tools-existing-jenkins.sh
```

### Option 2: Full Setup Script

```bash
# Clone the repository on your Jenkins server
git clone <repository-url> kobayashi_maru
cd kobayashi_maru

# Run setup script
./scripts/setup-jenkins-tools.sh

# Follow the interactive prompts
```

### Option 3: Manual Installation

```bash
# Make the provisioning script executable
chmod +x scripts/provision-jenkins-master.sh

# Run the provisioning script
./scripts/provision-jenkins-master.sh

# Reboot the system
sudo reboot
```

## What Gets Installed

### System Packages
- Build tools (GCC, Make, CMake, Ninja)
- ARM GCC Toolchain (13.2.rel1)
- Python 3 with development packages
- Git, curl, wget, unzip
- OpenJDK 17 for Jenkins
- Docker (for containerized builds)

### Embedded Development Tools
- **Renode** (1.15.0) - Robot simulation framework
- **ARM GCC Toolchain** - For FRDM-MCXN947 firmware compilation
- **OpenOCD** - For debugging and flashing
- **CAN utilities** - For CAN-FD development

### AI Unit Development (Optional)
- **Python** - For Raspberry Pi / Jetson development
- **Android SDK** - For Google Pixel development (optional)
- **ROS** - For advanced robotics integration (optional)
- **TensorFlow Lite / TensorRT** - For AI model deployment

### Python Packages
- numpy, scipy, matplotlib
- pyserial (for hardware communication)
- pytest (for testing)
- flake8 (for code analysis)

## Directory Structure

```
scripts/
├── provision-jenkins-master.sh       # Main provisioning script
├── setup-jenkins-tools.sh            # Automated setup script  
├── install-tools-existing-jenkins.sh # Quick installation for existing Jenkins
├── setup-jenkins-master.sh           # Lightweight Jenkins + tools setup
└── README.md                         # This file

docker/
├── Dockerfile                        # Container image (optional)
├── docker-compose.yml               # Container orchestration (optional)
└── .env                             # Environment variables

Jenkinsfile                           # Pipeline configuration
```

## Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `JENKINS_URL` | Existing Jenkins master URL | `http://192.168.50.65:8080` |
| `JENKINS_PORT` | Jenkins master port | `8080` |

## Jenkins Pipeline Configuration

The included `Jenkinsfile` provides:

- **Multi-stage build pipeline**
- **Firmware compilation** (ARM GCC)
- **AI unit app building** (if present)
- **Static analysis** (cppcheck, clang-tidy, flake8)
- **Unit testing**
- **Renode simulation testing**
- **Artifact archiving**

### Pipeline Stages

1. **Environment Setup** - Verify tools and environment
2. **Checkout** - Source code retrieval
3. **Build Firmware** - Compile embedded firmware
4. **Build AI Unit App** - Compile AI processing application (if present)
5. **Static Analysis** - Code quality checks
6. **Unit Tests** - Run test suites
7. **Renode Simulation Tests** - Hardware simulation tests
8. **Integration Tests** - System integration verification

## Jenkins Master Configuration

### For Existing Jenkins (192.168.50.65:8080)

1. **Install Tools**: Use one of the setup scripts above
2. **Create Pipeline Job**:
   - New Item → Pipeline
   - Pipeline script from SCM
   - Repository URL: Your Kobayashi Maru repository
   - Script Path: `Jenkinsfile`
3. **Pipeline runs on master node** (no separate agent needed)

## Running Renode Tests

### Manual Testing
```bash
cd simulation/renode
renode robot_simulation.resc

# In another terminal
telnet localhost 3456
```

### Automated Testing (in pipeline)
The Jenkins pipeline automatically runs Renode simulation tests with:
- 60-second timeout
- Communication interface testing
- Simulation model verification

## Troubleshooting

### Common Issues

1. **ARM GCC not found**:
   ```bash
   export PATH="/opt/arm-gnu-toolchain/bin:$PATH"
   # Or reboot to pick up /etc/environment changes
   ```

2. **Renode fails to start**:
   ```bash
   # Check mono installation
   mono --version
   
   # Run with verbose output
   renode --console --disable-xwt
   ```

3. **Jenkins can't find tools**:
   ```bash
   # Check Jenkins user environment
   sudo -u jenkins bash -c 'echo $PATH'
   
   # Restart Jenkins
   sudo systemctl restart jenkins
   ```

4. **Python packages missing**:
   ```bash
   # Install for Jenkins user
   sudo -u jenkins python3 -m pip install --user numpy scipy matplotlib pyserial pytest
   ```

### Performance Optimization

1. **Increase build parallelism**:
   ```bash
   make -j$(nproc)  # Use all CPU cores
   ```

2. **Jenkins heap size** (if needed):
   ```bash
   # Edit /etc/default/jenkins
   JAVA_ARGS="-Xmx4g -Xms2g"
   sudo systemctl restart jenkins
   ```

3. **SSD storage recommended** for faster builds

## Security Considerations

- Tools installed system-wide with sudo access
- Jenkins user configured with development environment
- Docker already available (version 28.5.1 detected)
- Network access on 192.168.50.65:8080

## Support

For issues related to:
- **Renode**: https://github.com/renode/renode
- **ARM Toolchain**: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain
- **Jenkins**: https://www.jenkins.io/doc/

## License

This deployment configuration is part of the Kobayashi Maru project and follows the same license terms.