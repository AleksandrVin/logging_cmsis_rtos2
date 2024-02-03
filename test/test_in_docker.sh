#!/bin/bash

set -e

SERIAL_DEVICE=/dev/pts/0

# start qemu in background 
qemu-system-arm -nographic -M stm32-f103c8 -kernel /app/firmware.bin -serial pty  &
QEMU_PID=$!

# wait until serial device is register and qemu is running
while [ ! -e $SERIAL_DEVICE ]; do
    if ! kill -0 $QEMU_PID 2>/dev/null; then
        echo "QEMU exited"
        exit 1
    fi
    sleep 0.1s
done

# cat $SERIAL_DEVICE

# print serial from /dev/pts/0 to console
python3 verify_output.py $SERIAL_DEVICE