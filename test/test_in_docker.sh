#!/bin/bash

set -e

ELF_FOR_GDB=/elf/firmware.elf
mkdir -p /elf
cp -v /app/firmware.elf $ELF_FOR_GDB
chmod a+r $ELF_FOR_GDB

# Default QEMU flags for normal test mode
QEMU_FLAGS=(
    "-nographic"
    "-M" "stm32-f103c8"
    "-kernel" "/app/firmware.bin"
    "-serial" "pty"
)

# Check if the first argument is "gdb"
if [ "$1" == "gdb" ]; then
    QEMU_FLAGS+=(
        "-gdb" "tcp::3333"
        "-S"
    )
    echo "Starting QEMU in GDB debug mode..."
else
    echo "Starting QEMU in normal test mode..."
fi

# Launch QEMU in background
qemu-system-arm "${QEMU_FLAGS[@]}" &
QEMU_PID=$!

SERIAL_DEVICE=/dev/pts/0

# Wait until serial device is registered and QEMU is running
while [ ! -e "$SERIAL_DEVICE" ]; do
    if ! kill -0 "$QEMU_PID" 2>/dev/null; then
        echo "QEMU exited unexpectedly."
        exit 1
    fi
    sleep 0.1s
done

# Create logs directory
mkdir -p /test/logs
chmod a+rw /test/logs

# If running in debug mode, notify the user to connect GDB
if [ "$1" == "gdb" ]; then
    echo "QEMU is waiting for GDB to connect on port 3333..."
fi

# Run the output verification script
python3 verify_output.py "$SERIAL_DEVICE"

# Wait for QEMU to exit
wait "$QEMU_PID"
