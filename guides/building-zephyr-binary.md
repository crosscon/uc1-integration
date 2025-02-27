# Building Zephyr binary

Note: This step can be omitted if the flashing binary was prepared in advance.

This section describes building the Zephyr binary on the "hello world" example.

1. Clone Zephyr repository:

    ```bash
      git clone https://github.com/zephyrproject-rtos/zephyr
    ```

1. Enter the directory:

    ```bash
      cd zephyr
    ```

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

1. Initialize `west`:

    ```bash
      west init
    ```

1. Update `west`:

    ```bash
      west update
    ```

1. Build the project of choice:

    ```bash
      west build -b lpcxpresso55s69/lpc55s69/cpu0 samples/hello_world
    ```

After building, the binary files will be available under `zephyr/build`. We will
need the `zephyr.bin` for flashing.
