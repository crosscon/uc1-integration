# crosscon-uc1-1

## Prepare the Environment

- Install [docker](https://docs.docker.com/engine/install/fedora/)
- Install [west](https://docs.zephyrproject.org/latest/develop/west/install.html)

## Initialize Workspace

```bash
mkdir workspace
cd workspace
git clone https://github.com/crosscon/uc1-integration
git checkout $BRANCH # if testing from branch other than main
cd uc1-integration
git submodule update --init --recursive
west init -l && west update
```

## Usage

Refer to the `uc1.sh` for details of available options. Some useful scenarios
are described below.

Default `USART` assignment:
- `USART0` - hypervisor
- `USART2` - VM1 (Zephyr)
- `USART3` - VM0 (bare-metal app, or Zephyr in PUF integration)

> **Note:**
> By default Zephyr uses USART2 for console output. It can be changed to UART3
> via overlay in app directory. Refer to the `hello_world_vm0` or `wifi_app`
> apps.

### UC1.1 integration

```bash
/uc1.sh build_puf && ./uc1.sh flash && ./uc1.sh hv_start
```

### Bare-metal + Zephyr VMs

```bash
export ZEPHYR_APP="timer_test"
export HV_CONFIG="two_bm_zephyr"
./uc1.sh build && ./uc1.sh flash && ./uc1.sh hv_start
```

### Single VM

```bash
export ZEPHYR_APP="wifi_app"
export HV_CONFIG="single_zephyr"
./uc1.sh build && ./uc1.sh flash && ./uc1.sh hv_start
```

### Bare-metal Zephyr app (without HV)

```bash
export ZEPHYR_APP="wifi_app"
./uc1.sh no_hv_zephyr
```

## Configure the WiFi network

1. Go to `app/src/main.c`
2. In lines:

    ```C
    #define WIFI_SSID "rpi3-hotspot"
    #define WIFI_PASSWORD "rpi3-pass"
    ```

    Set the values to actual SSID and password of the target wifi network.

3. Save the file.
