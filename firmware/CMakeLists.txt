cmake_minimum_required(VERSION 3.20)

# Top-level firmware CMake now delegates to the Zephyr app.
# This CMakeLists intentionally does not build the legacy QP-based firmware.

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/zephyr_app/CMakeLists.txt")
    message(STATUS "Building Zephyr app from firmware/zephyr_app")
    add_subdirectory(zephyr_app)
else()
    message(FATAL_ERROR "zephyr_app CMakeLists.txt not found. Nothing to build.")
endif()
