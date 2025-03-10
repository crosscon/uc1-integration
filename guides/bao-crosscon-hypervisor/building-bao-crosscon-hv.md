# Build hypervisor

## Preparation

1. Clone `https://github.com/danielRep/bao-hypervisor`:

    ```bash
    git clone https://github.com/danielRep/bao-hypervisor
    ```

2. Copy `config.c` to `bao-hypervisor/configs/lpc55.c`

    ```bash
    cp ./config.c ./bao-hypervisor/configs/lpc55.c
    ```

3. Change branch in `bao-hypervisor` to `exp/armv8m-sched`:

    ```bash
    cd bao-hypervisor
    git checkout exp/armv8m-sched
    ```

4. Apply patch `makefile-patch.diff`:

    ```bash
    git apply ../makefile-patch.diff
    ```

## Build container

```bash
docker build . -t bao-hypervisor-image
```

## Run container

```bash
docker run -it -v ./bao-hypervisor:/workspace/bao-hypervisor bao-hypervisor-image bash
```

The bao-hypervisor repository from the local directory available from inside the
container.

## Build the binary

From inside the container, run:

  ```bash
  cd bao-hypervisor
  make clean
  export CROSS_COMPILE=arm-none-eabi-
  export PLATFORM=lpc55s69
  export CONFIG=lpc55
  make
  ```

## Result

Resulting binaries can be found at: `bao-hypervisor/bin`
