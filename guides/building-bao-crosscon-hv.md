# Build hypervisor

This document guides through building the bao hypervisor, two example vm
applications, vm0 and vm1, and assembling them into one binary, which can be
flashed on the platform.

## Preparation

1. Clone repositories

    ```bash
    git clone https://github.com/danielRep/bao-hypervisor
    git clone https://github.com/danielRep/bao-baremetal-guest
    ```

2. Build the container:

    ```bash
    docker build resources/building-bao-hypervisor/ -t bao-hypervisor-image
    ```

3. Run container shell:

    ```bash
    docker run -it -v .:/workdir/ bao-hypervisor-image bash
    ```

4. Create the target directory

    ```bash
    mkdir /workdir/binaries
    ```

5. Enter the workdir

    ```bash
    cd /workdir/binaries
    ```

## Build the Hypervisor

1. Enter the directory:

    ```bash
    cd bao-hypervisor
    ```

2. Copy `config.c` to `bao-hypervisor/configs/lpc55.c`

    ```bash
    cp ../resources/building-bao-hypervisor/config.c ./configs/lpc55.c
    ```

3. Change branch in `bao-hypervisor` to `exp/armv8m-sched`:

    ```bash
    git checkout exp/armv8m-sched
    ```

    * If above command fails with message below, please follow instruction
      provided by git:

      ```bash
      fatal: detected dubious ownership in repository at '/workspace/bao-hypervisor'
      ```

4. Apply patch `makefile-patch.diff`:

    ```bash
    git apply ../resources/building-bao-hypervisor/makefile-patch.diff
    ```

5. Build the hypervisor binary:

    ```bash
    make clean && make PLATFORM=lpc55s69 CONFIG=lpc55 DEBUG=y
    ```

    Resulting binary can be found at: `bao-hypervisor/bin/bao.bin`

6. Copy binary to workspace as `bao.bin`:

    ```bash
    cd ./bin/lpc55s69/lpc55/
    cp bao.bin bao.elf /workdir/binaries/
    cd -
    ```

## Build applications

1. Enter the `bao-baremetal-guest` directory

    ```bash
    cd bao-baremetal-guest
    ```

2. Change branch to `exp/armv8m-ipc`:

    ```bash
    git checkout exp/armv8m-ipc
    ```

    * If above command fails with message below, please follow instruction
      provided by git:

      ```bash
      fatal: detected dubious ownership in repository at '/workspace/bao-baremetal-guest'
      ```

3. Build the first application:

    ```bash
    make clean && make PLATFORM=lpc55s69 GUEST=0
    ```

    Resulting binaries can be found at: `build/lpc55s69/`

4. Copy vm0 binaries to the workspace:

    ```bash
    cd ./build/lpc55s69/
    cp baremetal.bin /workdir/binaries/vm0.bin
    cp baremetal.elf /workdir/binaries/vm0.elf
    cd -
    ```

5. Build the second application:

    ```bash
    make clean && make PLATFORM=lpc55s69 GUEST=1
    ```

    Resulting binaries can be found at: `build/lpc55s69/`

6. Copy vm1 binaries to the workspace:

    ```bash
    cd ./build/lpc55s69/
    cp baremetal.bin /workdir/binaries/vm1.bin
    cp baremetal.elf /workdir/binaries/vm1.elf
    cd -
    ```

## Flash binaries on the board

This section covers flashing the resulting binaries on the board, with either
LinkServer and lpc55 tools.

The following actions should be performed in the `binaries/` dir:

```bash
cd /workdir/binaries
```

### Flash with lpc55

The `lpc55` tool is to be used on the ARM-based platforms, such as RPI.

#### Assemble final binary

At this point, all 3 binaries should be available in the workspace directory:

1. `bao.bin`

2. `vm0.bin`

3. `vm1.bin`

To create a final binary that can be flashed onto the board:

```bash
dd if=/dev/zero of=demo.bin bs=1k count=512 &&
dd if=bao.bin of=demo.bin bs=1 seek=0 conv=notrunc &&
dd if=vm0.bin of=demo.bin bs=1 seek=131072 conv=notrunc &&
dd if=vm1.bin of=demo.bin bs=1 seek=262144 conv=notrunc
```

The resulting file `demo.bin` will contain:

1. Hypervisor (from `bao.bin`) at 0x0

2. VM0 (from `vm0.bin`) at 0x00020000 (131072 in decimal)

3. VM1 (from `vm1.bin`) at 0x00040000 (262144 in decimal)

#### Flash assembled binary

1. Check available bootloaders:

    ```bash
    lpc55 ls
    ```

2. Flash the binary:

    ```bash
    lpc55 write-flash demo.bin
    ```

### Flash with LinkServer

The `LinkServer` tool is to be used only on x86 platforms.

To flash with LinkServer, instead of `.bin`, all 3 `.elf` files are required.

1. Check probes:

    ```bash
    LinkServer probes
    ```

    Example output:

    ```bash
      #  Description                 Serial    Device    Board
     -------
      1  LPC-LINK2 CMSIS-DAP V5.460  ORAQBQIR
    ```

2. Flash the `.elf` files:

    ```bash
    LinkServer flash LPC55S69:LPCXpresso55S69 load bao.elf
    LinkServer flash LPC55S69:LPCXpresso55S69 load vm0.elf
    LinkServer flash LPC55S69:LPCXpresso55S69 load vm1.elf
    ```
