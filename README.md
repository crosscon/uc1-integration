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

### TLS client app

`tls_client` app integrates both WiFi and TLS client features. It is a basis
for the final demonstration.

At the moment, TLS client app needs a few extra steps:

* Copy `resources/wolfssl/user_settings.h` to `wolfssl` module:

> TODO: There must be a better way of providing these settings, directly from
> our repo?

```bash
cp resources/wolfssl/user_settings.h ../modules/crypto/wolfssl
```

* Increase flash partition size in `zephyr`:

```diff
diff --git a/boards/nxp/lpcxpresso55s69/lpcxpresso55s69.dtsi b/boards/nxp/lpcxpresso55s69/lpcxpresso55s69.dtsi
index b05177ee3c70..d8ed9cb19db9 100644
--- a/boards/nxp/lpcxpresso55s69/lpcxpresso55s69.dtsi
+++ b/boards/nxp/lpcxpresso55s69/lpcxpresso55s69.dtsi
@@ -128,7 +128,7 @@
                };
                slot0_partition: partition@8000 {
                        label = "image-0";
-                       reg = <0x00008000 DT_SIZE_K(96)>;
+                       reg = <0x00008000 DT_SIZE_K(320)>;
                };
                slot0_ns_partition: partition@48000 {
                        label = "image-0-nonsecure";
```

* Build and run `server_tls` from `crosscon-uc1-2` repo (not published here
yet). Perhaps should be integrated in here?

* Set `SERVER_ADDR` in `tls_client/src/main.c` matching to the IP address of
the machine where `server_tls` has been started

* Build and run `tls_client`:

```bash
export ZEPHYR_APP="tls_client"
./uc1.sh no_hv_zephyr
```

## Configure the WiFi network

Local changes to the WiFi Settings can be made via
`wifi_app/src/wifi_config_local.h` in WiFi-enabled apps directories. These
settings are covered in `.gitignore`, so there is no need to worry about
leaking them to repo.
