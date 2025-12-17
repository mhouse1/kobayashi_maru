This directory will contain FreeRTOS kernel source files and configuration for the project. Add the FreeRTOS kernel (typically from https://github.com/FreeRTOS/FreeRTOS-Kernel) and a FreeRTOSConfig.h tailored for your MCU and application.

Steps:
1. Download FreeRTOS-Kernel source (portable, include, and kernel files).
2. Place them in this directory or subdirectories as appropriate.
3. Add a FreeRTOSConfig.h file (see FreeRTOS documentation for template).
4. Update CMakeLists.txt to include these sources in the build.
