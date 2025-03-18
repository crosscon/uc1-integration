# Build hypervisor

## Preparation

1. Clone repositories

    ```bash
    git clone https://github.com/danielRep/bao-hypervisor
    git clone https://github.com/danielRep/bao-baremetal-guest
    ```

2. Build the container:

    ```bash
    docker build . -t bao-hypervisor-image
    ```

3. Run container shell:

    ```bash
    docker run -it -v .:/workspace/ bao-hypervisor-image bash
    ```

## Build the Hypervisor

1. Enter the directory:

    ```bash
    cd bao-hypervisor
    ```

2. Copy `config.c` to `bao-hypervisor/configs/lpc55.c`

    ```bash
    cp ../config.c ./configs/lpc55.c
    ```

3. Change branch in `bao-hypervisor` to `exp/armv8m-sched`:

    ```bash
    git checkout exp/armv8m-sched
    ```

4. Apply patch `makefile-patch.diff`:

    ```bash
    git apply ../makefile-patch.diff
    ```

5. Build the hypervisor binary:

    ```bash
    cd bao-hypervisor
    make clean
    export CROSS_COMPILE=arm-none-eabi-
    export PLATFORM=lpc55s69
    export CONFIG=lpc55
    make
    ```

    Resulting binary can be found at: `bao-hypervisor/bin/bao.bin`

6. Copy binary to workspace as `bao.bin`:

    ```bash
    cp ./bin/bao.bin ../bao.bin
    ```

## Build applications

1. Enter the `bao-baremetal-guest` directory

    ```bash
    cd bao-baremetal-guest
    ```

2. Change branch to `exp/armv8m\_20000` (VM0):

    ```bash
    git checkout exp/armv8m\_20000
    ```

3. Build the application:

    ```bash
    make clean
    export CROSS_COMPILE=arm-none-eabi-
    export PLATFORM=lpc55s69
    export CONFIG=lpc55
    make
    ```

    Resulting binary can be found at: `build/lpc55s69/baremetal.bin`

4. Copy binary to workspace as `vm0.bin`:

    ```bash
    cp ./build/lpc55s69/baremetal.bin ../vm0.bin
    ```

5. Change branch to `exp/armv8m\_40000` (VM1):

    ```bash
    git checkout exp/armv8m\_40000
    ```

6. Build the application:

    ```bash
    make clean
    export CROSS_COMPILE=arm-none-eabi-
    export PLATFORM=lpc55s69
    export CONFIG=lpc55
    make
    ```

    Resulting binary can be found at: `build/lpc55s69/baremetal.bin`

7. Copy binary to workspace as `vm1.bin`:

    ```bash
    cp ./build/lpc55s69/baremetal.bin ../vm1.bin
    ```

## Assemble final binary

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
