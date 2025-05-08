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
cd crosscon-uc1-1
west init -l && west update
```

## Usage

Refer to the `uc1.sh` for details.

```bash
export ZEPHYR_APP="timer_test"
export HV_CONFIG="two_bm_zephyr"
./uc1.sh build && ./uc1.sh flash && ./uc1.sh hv_start
```

Default `USART` assignment:
- `USART0` - hypervisor
- `USART2` - Zephyr
- `USART3` - bare-metal app

> **Note:**
> `wifi_app` will need `USART2` for communication with WiFi module. An
> alternative Zephyr code using `USART3` as Zephyr console for debugging
> capabilities of this app is available
> [here](https://github.com/3mdeb/zephyr/tree/bao-ipc-flexcomm3-usart).
> This can be selected in `west.yml` file.

## Configure the WiFi network

1. Go to `app/src/main.c`
2. In lines:

    ```C
    #define WIFI_SSID "rpi3-hotspot"
    #define WIFI_PASSWORD "rpi3-pass"
    ```

    Set the values to actual SSID and password of the target wifi network.

3. Save the file.
