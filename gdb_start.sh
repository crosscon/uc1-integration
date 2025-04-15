#!/usr/bin/env bash

if [ ! -f "/.dockerenv" ]; then
  echo "This script is expected to run in bao HV container"
  exit 1
fi

# Start LinkServer gdbserver in background
LinkServer gdbserver LPC55S69:LPCXpresso55S69 &
GDBSERVER_PID=$!

# Give the server a moment to start
sleep 3

# TODO: load modules automatically?
# Run GDB commands non-interactively
arm-none-eabi-gdb out/bao.elf \
  -ex "target remote :3333"

# After GDB exits, kill the gdbserver
kill $GDBSERVER_PID
