# Debugging bao crosscon hypervisor with `arm-none-eabi-gdb` and `LinkServer`

This guide explains how to debug the crosscon hypervisor application with
`arm-none-eabi-gdb` debugger and `LinkServer` server. It also guides through
preparing the necessary reusable environment (docker container) for this
purpose.

## Preparing environment (container)

The container used for building the bao hypervisor
(`resources/building-bao-hypervisor/Dockerfile`) already contains all required
dependencies for debugging.

1. Build the container

    ```bash
    docker build resources/building-bao-hypervisor/ -t bao-hypervisor-image
    ```

2. Start the container in interactive shell

    ```bash
    docker run --rm -it --privileged -v /dev:/dev -v .:/workdir bao-hypervisor-image bash
    ```

Note: it is required to prepare the file `baremetal.elf` from building the `bao`
hypervisor as `bao.elf` in the current directory. It will be available from the
container.

## Flashing required firmware

1. Connect `DFU` jumper (`J4`) on lpcxpresso board.

2. Connect board to PC through USB `P6`

3. Check USB devices with `lsusb` :

    ```bash
    lsusb | grep -i nxp
    Bus 003 Device 028: ID 1fc9:000c NXP LPC
    ```

    Important: The command should output this exact device - `NPX LPC`.

4. Check the devices with `dfu-util`:

    ```bash
    root@4af2052b85d9:/work# dfu-util -l
    dfu-util 0.9

    Copyright 2005-2009 Weston Schmidt, Harald Welte and OpenMoko Inc.
    Copyright 2010-2016 Tormod Volden and Stefan Schmidt
    This program is Free Software and has ABSOLUTELY NO WARRANTY
    Please report bugs to http://sourceforge.net/p/dfu-util/tickets/

    Found DFU: [1fc9:000c] ver=0100, devnum=28, cfg=1, intf=0, path="3-1", alt=0, name="DFU", serial="ABCD"
    ```

5. Program the `CMSIS-DAP` firmware:

    To program the `CMSIS-DAP` firmware, run the `program_CMSIS` script:

    ```bash
    /usr/local/LinkServer/lpcscrypt/scripts/program_CMSIS 
    ```

    Note: This process may take around 2 minutes.

    The result:

    ```bash
    LPCScrypt - CMSIS-DAP firmware programming script v2.1.3 Jul 2023.

    Connect an LPC-Link2 or LPCXpresso V2/V3 Board via USB then press Space.
    Booting LPCScrypt
    .


    CMSIS-DAP firmware successfully programmed to flash.

    LPCXpresso V2/V3 programmed with LPC432x_IAP_CMSIS_DAP_V5_460.bin and has the uniqueID: ORAQBQIR
      - To use: remove DFU link and reboot the board

    Connect next board then press Space (or <return> to Quit)
    ```

    As the program indicates, press `<return>` (Enter) to quit the program.

6. Remove the `DFU` jumper (`J4`) jumper from the board. Power cycle the board.

7. Check the board with `lsusb`:

    ```bash
    root@e18aebda6c80:/work# lsusb | grep -i nxp
    Bus 003 Device 036: ID 1fc9:0090 NXP Semiconductors LPC-LINK2 CMSIS-DAP V5.460
    ```

8. Check available probes:

    ```bash
    root@e18aebda6c80:/work# LinkServer probes
      #  Description                 Serial    Device    Board
    ---  --------------------------  --------  --------  -------
      1  LPC-LINK2 CMSIS-DAP V5.460  ORAQBQIR
    ```

9. Start `gdbserver` as a background process:

    ```bash
    root@e18aebda6c80:/work# LinkServer gdbserver LPC55S69:LPCXpresso55S69 &
    INFO: Exact match for LPC55S69:LPCXpresso55S69 found
    INFO: Selected device LPC55S69:LPCXpresso55S69
    INFO: Getting available probes
    INFO: Selected probe #1 ORAQBQIR (LPC-LINK2 CMSIS-DAP V5.460)
    GDB server listening on port 3333 in debug mode (core cm33_core0)
    Semihosting server listening on port 4444 (core cm33_core0)
    INFO: [stub (3333)] Ns: LinkServer RedlinkMulti Driver v24.12 (Dec 18 2024 18:40:01 - crt_emu_cm_redlink build 869)
    ```

    Wait about 5 seconds for it to setup.

10. Run `arm-none-eabi-gdb`:

    Note: `gdb` is part of the same toolchain that is already used for building
    applications for the LPC - see the Dockerfile for building bao hypervisor
    and bare-metal applications.

    ```bash
    arm-none-eabi-gdb
    ```

    In the `gdb` shell, run:

    ```bash
    (gdb) target remote :3333
    (gdb) file bao.elf
    (gdb) set $pc=_reset_handler
    (gdb) c 
    ```

### References

- Dockerfile - minimized version from:
  <https://git.3mdeb.com/3mdeb/LPCxpresso55S69_dev_container>

- LinkServer:
  <https://www.nxp.com/design/design-center/software/development-software/mcuxpresso-software-and-tools-/linkserver-for-microcontrollers:LINKERSERVER>\

- UM11158 LPCXpresso55S69/55S28 Development Boards - jumper settings, etc

- NK documentation:
  <https://github.com/Nitrokey/nitrokey-3-firmware/blob/main/docs/lpc55-quickstart.md#debugging>

- references to:
  <https://www.nxp.com/docs/en/supporting-information/Debug_Probe_Firmware_Programming.pdf>
  which explains JLINK vs CMSIS-DAP (Chapter `4. Debug Firmware Variants and
  Drivers`) and how the debug probe should be programmed (chapter `2.
  QuickStart`)

- `arm-none-eabi` toolchain:
  <https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz>
