# Building Zephyr binary

Note: This step can be omitted if the flashing binary was prepared in advance.

This section describes building the Zephyr binary on the "hello world" example.

## Prerequisites

1. Install `west` tool:

    ```bash
      pip3 install --user -U west
    ```

1. Install Zephyr SDK (follow
[guide](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html)):

1. (Optional) If `elftools` is missing, install them:

    ```bash
      pip3 install --user pyelftools
    ```

## Build

1. Initialize west:

    ```bash
      west init -m https://github.com/danielRep/zephyr.git
    ```

1. Checkout the branch:

    ```bash
    cd zephyr/
    git checkout feat/bao-ipc
    cd ..
    ```

1. Update `west`:

    ```bash
      west update
    ```

1. Build the project of choice:

    ```bash
      west build -b lpcxpresso55s69/lpc55s69/cpu0/ns zephyr/samples/hello_world -p
    ```

(Optional) If building the project results in an error, try following those
steps:

1. Create virtual environment

    ```bash
    python -m venv .venv
    source .venv/bin/activate
    ```

1. Install `pipreqs`

    ```bash
    pip install pipreqs
    ```

1. Generate `requirements.txt`

    ```bash
    pipreqs --savepath requirements.txt --force ./zephyr/
    ```

1. Install dependencies from `requirements.txt`

    ```bash
    pip install -r requirements.txt
    ```

After building, the binary files will be available under `zephyr/build`. We will
need the `zephyr.elf` for flashing.

1. Copy binary

```bash
cp ./build/zephyr/zephyr.elf ../binaries
```

1. (Optional) Verify entrypoint for the application

```bash
$ readelf -aW ../binaries/zephyr.elf | grep __start
085: 00040c1d     0 FUNC    GLOBAL DEFAULT    2 __start
```

In the bao hypervisor config, the `.entry` should be the same address, minus 1.
