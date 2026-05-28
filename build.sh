#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

# =============================================================================
# Build Configuration Facade
# =============================================================================
BASE_FILES="main.cpp"
COMMON_FLAGS="-nostdlib -fno-exceptions -fno-rtti -fno-stack-protector -fno-builtin -O3"
OUTPUT_NAME="coeperunt-austri"

# =============================================================================
# Target Platform Auto-Detection
# =============================================================================
OS_NAME="$(uname -s)"
echo "----------------------------------------"
echo "Target Platform Detection..."
echo "Detected Host OS: $OS_NAME"

if [[ "$OS_NAME" == *"MINGW"* || "$OS_NAME" == *"MSYS"* || "$OS_NAME" == *"CYGWIN"* ]]; then
    echo "Configuring build for Windows (PE Binary)..."
    LINKER_FLAGS="-Wl,-entry:no_crt_entry -Wl,-subsystem:windows -lkernel32 -luser32"
    OUTPUT_BIN="${OUTPUT_NAME}.exe"
else
    echo "Configuring build for Linux (ELF Binary)..."
    LINKER_FLAGS="-Wl,-e,_start -lX11 -lGL -lc"
    OUTPUT_BIN="${OUTPUT_NAME}"
fi
echo "----------------------------------------"

# =============================================================================
# Unified Compilation Execution
# =============================================================================
echo "Building target: ${OUTPUT_BIN}"
echo "Source Files:    ${BASE_FILES}"
echo "Flags:           ${COMMON_FLAGS} ${LINKER_FLAGS}"

clang++ ${COMMON_FLAGS} ${LINKER_FLAGS} -o "${OUTPUT_BIN}" ${BASE_FILES}

echo "----------------------------------------"
echo "SUCCESS: Created ${OUTPUT_BIN}"
echo "----------------------------------------"
