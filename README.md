# crosscon-uc1-1

## Prepare the Environment

- Install [docker](https://docs.docker.com/engine/install/fedora/)
- Install [west](https://docs.zephyrproject.org/latest/develop/west/install.html)

## Initialize Workspace

```bash
mkdir workspace
cd workspace
git clone https://github.com/crosscon/uc1-integration
cd uc1-integration
git checkout $BRANCH # if testing from branch other than main
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

* Increase flash partition size in `../zephyr`:

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

### Two VMs: mtls client and puf

This section shows how to create a setup with mtls client and puf VMs.

1. Set up wifi credentials in `tls_client/src/wifi_config_local.h` and update
   IP address of tls server app (`SERVER_ADDR`) in `tls_client/src/main.c `.

1. Copy app overlay to puf_vm1.

    ```bash
    cp resources/overlays/mtls_puf_vm1.overlay puf_vm1/application/app.overlay
    ```

1. Decrease `time_slice` to 1ms in hypervisor source code.
    ```bash
    sed -i 's/TIME_MS(10))/TIME_MS(1))/' CROSSCON-Hypervisor/src/core/sched.c
    ```
    Note: This is a workaround for UART overflow to fix issues with wifi
    card. It gives more opportunities for Zephyr VM to copy UART data from UART
    FIFO to buffer.
    [[Source](https://github.com/crosscon/CROSSCON-Hypervisor-and-TEE-Isolation-Demos/issues/37)]

1. Run following command to build, flash and run the demo

    ```bash
    ./uc1.sh build_mtls_puf && ./uc1.sh flash && ./uc1.sh hv_start
    ```

## Configure the WiFi network

Local changes to the WiFi Settings can be made via
`<app_directory>/src/wifi_config_local.h` in WiFi-enabled apps directories.
These settings are covered in `.gitignore`, so there is no need to worry about
leaking them to repo.

WiFi-enabled apps are for instance:
- `wifi_app`,
- `tls_client`.

This file should follow the following format:

```
#define WIFI_SSID      "SSID"
#define WIFI_PASSWORD  "PASSWORD"
```

## Running sagemath scripts

There is no `sagemath` for Fedora, but there is public container image. Once the
image is fetched, one can run the following command to run sagemath scripts to
validate PUF authenticity.

```bash
docker run -it -v $(pwd)/puf_vm1/scripts/proofs/:/mnt sagemath/sagemath sage /mnt/proof_verifier_calc.sage -i
```

The above command will run the script in interactive mode.
Below is command example of non-interactive mode

```bash
docker run -it -v $(pwd)/puf_vm1/scripts/proofs/:/mnt sagemath/sagemath sage /mnt/proof_verifier_calc.sage \
-gx 0x6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296 \
-gy 0x4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5 \
-hx 0xd13353e86b41f94c8877f68fb95aad0a35820695e2037413bd57a9c447df11d9 \
-hy 0xa248caebbb366b69fdebd312588b9702d81de34eed740ed27a246d2ee7ba43e4 \
-COMx 0x76268a64a89d4c78c2583af88854c2dfae6ab162af1b47d4fa49d0d4e3817393 \
-COMy 0x1e0fb6b50eec5238733969c4ed418b89ff7e3ee593ffb36baae3bdfd09cdb56e \
-Px 0x6671490b610f21c07146091c168927a0b218ac91695ff531d9f40e430be41123 \
-Py 0x94bfc6c6f2ba416717469207a4a43ce7e463b93b65d81164d50f1bf8c058acbe \
-v 0x2f848af2e2f055baf62239b0042e18c09b7bc6fd9092264d0a4b5004cfeb3114f10de68bab6552f4da0450f7636f331b9b29f95b92cd1e6e9e0baf09d1df3e16 \
-w 0x18fef11fbd03c5975b7e01c36a10fee0f531831ecfe60772f763e43de1cd3c41233cca6667d6b650f2b3ef6b1a5a13bb62861191efc33b17458810ba80f0e5da \
-n 0X8899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF0011223344556677
```

There is also the script for generating sagemath command based on the PUF output

```bash
python3 resources/scripts/cnvrt_puf_output.py <input_file>
```

_Note: Nonce is 64 bytes long._
