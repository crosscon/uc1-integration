# crosscon-uc1-1

## Prepare the Environment

- Follow [Zephyr Getting
  Started](https://docs.zephyrproject.org/latest/develop/getting_started/installation_linux.html#installation-linux)
- Install [Zephyr
  SDK](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html#install-zephyr-sdk-on-linux)
- Install
  [west](https://docs.zephyrproject.org/latest/develop/west/install.html)

## Initialize Workspace

```bash
mkdir workspace
cd workspace
git clone ssh://git@git.3mdeb.com:2222/3mdeb/crosscon-uc1-1.git
git checkout $BRANCH # if testing from branch other than main
cd crosscon-uc1-1
west init -l && west update
```

## Usage

Refer to the `uc1.sh` for details.

```bash
./uc1.sh build && ./uc1.sh flash && ./uc1.sh hv_start
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

## Build the application

```bash
west build -b lpcxpresso55s69/lpc55s69/cpu0 --shield mikroe_wifi_bt_click_mikrobus app --pristine
```
