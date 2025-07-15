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

# Run GDB commands non-interactively
arm-none-eabi-gdb out/crossconhyp.elf \
  -ex "set pagination off" \
  -ex "set confirm off" \
  -ex "target remote :3333" \
  -ex "set \$pc=_reset_handler" \
  -ex "set confirm on" \
  -ex "continue" \
  -ex "ref" \
  -ex "quit"

# Note, you can also load the zephyr debug symbols via following commands
# -ex "add-symbol-file out/vm0.elf 0x00020000" \
# -ex "add-symbol-file out/vm1.elf 0x00048000" \
# Update the VM address and add these before "set confirm on"

# After GDB exits, kill the gdbserver
kill $GDBSERVER_PID
