cmake_minimum_required(VERSION 3.20)

# Toolchain must be set before project() when cross-compiling
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ARM toolchain settings
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_SIZE arm-none-eabi-size)
find_program(CMAKE_MAKE_PROGRAM make)

# Project configuration
project(robot_firmware VERSION 1.0.0 LANGUAGES C CXX ASM)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# MCU Configuration for FRDM-MCXN947 (Cortex-M33)
set(MCU_FLAGS 
    "-mcpu=cortex-m33"
    "-mthumb"
    "-mfloat-abi=hard"
    "-mfpu=fpv5-sp-d16"
)

# Compiler flags
add_compile_options(${MCU_FLAGS})
add_compile_options(-Wall -Wextra -fdata-sections -ffunction-sections)

# C++ specific flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Linker flags
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/mcxn947.ld")
add_link_options(${MCU_FLAGS})
add_link_options(-T${LINKER_SCRIPT} -Wl,--gc-sections -Wl,-Map=${PROJECT_NAME}.map -specs=nano.specs -specs=nosys.specs)

# Debug/Release specific flags
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

# Source files


# FreeRTOS kernel sources
file(GLOB FREERTOS_KERNEL_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/freertos/kernel/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/*.c"
)

# QP/C core sources
file(GLOB QPC_CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/qpc/src/qf/*.c"
)

# QP/C FreeRTOS port sources
set(QPC_PORT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/qpc/ports/freertos/qf_port.c
    ${CMAKE_CURRENT_SOURCE_DIR}/qpc/ports/freertos/syscalls.c
)

# Project C sources (remove C++ sources)
file(GLOB_RECURSE C_SOURCES "src/*.c" "src/**/*.c")

# Create executable
add_executable(${PROJECT_NAME}.elf
    ${C_SOURCES}
    ${FREERTOS_KERNEL_SOURCES}
    ${QPC_CORE_SOURCES}
    ${QPC_PORT_SOURCES}
    src/startup_mcxn947.c
)

# Include directories
target_include_directories(${PROJECT_NAME}.elf PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos/kernel/include
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos
    ${CMAKE_CURRENT_SOURCE_DIR}/qpc/include
    ${CMAKE_CURRENT_SOURCE_DIR}/qpc/ports/freertos
)


## QP/C++ framework removed for FreeRTOS/C migration

# Post-build commands to generate binary formats
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
    COMMENT "Generating hex and bin files, showing size..."
)

# Install targets
install(FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.elf
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)

# Print build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Target: FRDM-MCXN947 (Cortex-M33)")
