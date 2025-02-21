# crosscon-uc1-1

## Prepare the Environment

- Follow [Zephyr Getting Started](https://docs.zephyrproject.org/latest/develop/getting_started/installation_linux.html#installation-linux)
- Install [Zephyr SDK](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html#install-zephyr-sdk-on-linux)
- Install [west](https://docs.zephyrproject.org/latest/develop/west/install.html)

## Initialize Workspace

```bash
west init -l && west update
```

## Build the application

```bash
west build -b lpcxpresso55s69/lpc55s69/cpu0 app --pristine
```
