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

### UC1.1 Integration (TLS client + PUF VM)

This section instructs on how to build the final demo for Use Case 1.1. As a
result of the commands listed below, a hypervisor with mutual tls client zephyr
VM and PUF VM will be deployed and run on the LPC55S69 platform.

**FIX ME: Add link to workflow/components diagram**

#### Prerequisites

Prior to building up this demo (mtls client), one shall first build and deploy
tls server application. The detailed manual
[can be found here](https://git.3mdeb.com/3mdeb/crosscon-uc1-2/src/commit/5c429a126d25c22dc9d9ce95838265e7640324ac/include/local_challenge.c#L32).


**FIX ME: Fix link when PR is merged.**

Make sure the common codebase has been synced, a `tls_client/src/common` should
be created and have contents.

#### Building and flashing TLS Client + PUF

1. Copy `resources/wolfssl/user_settings.h` to `wolfssl` module:

   ```bash
   cp resources/wolfssl/user_settings.h ../modules/crypto/wolfssl
   ```

1. [Set up wifi credentials](#configure-the-wifi-network) in
   `tls_client/src/wifi_config_local.h` and update IP address of tls server app
   (`SERVER_ADDR`) in `tls_client/src/main.c `.

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

1. Run following command to build, flash and run the demo. Note that the
   application will work only for **pre-enrolled targets**. The details on how
   to enroll the target can be found [here](resources/enrollment_bin/readme.md).
   The target number directly corresponds to platform asset number. Currently
   supported targets are:
   * 246

   The command for building is as follows:

    ```bash
    export TARGET=<target_no>; ./uc1.sh build_mtls_puf && ./uc1.sh flash
    ```

#### Running the demo

To run the demo simply execute the following:

```bash
/uc1.sh hv_start
```

The command will run gdb to kickstart the hypervisor.

#### Building UWU demo

To build GUEST_VM + PUF_VM, a reference, legacy demo supplied by UWU, run the
following command. Note that the legacy demo does not use WIFI module.

```bash
export TARGET=<target_no>; /uc1.sh build_puf && ./uc1.sh flash && ./uc1.sh hv_start
```

If the demo succeeds, on the server side you shall see the following:

``` log
✅ Proof verifies: g^v·h^w = P·COM^α
Computation complete
Client: Hello from Zephyr!

Shutdown complete
Waiting for a connection...
```

... and on the client side...

```log
[00:00:51.036,000] <inf> MAIN: Received from server: Hello from WolfSSL TLS server

[00:00:51.036,000] <inf> MAIN: Starting SSL shutdown...
```

## Misc

### Configure the WiFi network

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

### Running sagemath scripts

#### Introduction

The `sagemath` scripts supplied with the VMs are source of trust for verifying
PUF authenticity. These can be used for comparison, between
[the C implementation](https://git.3mdeb.com/3mdeb/crosscon-uc1-2/pulls/10/files#diff-9254371f1043cddfdbcb224e2308086a10512625).

**FIX ME: Update link to file instead to PR when merged.**

#### Running sagemath

There is no `sagemath` for Fedora, but there is
[public docker image available](https://hub.docker.com/r/sagemath/sagemath).
Once the image is fetched, one can use the following set of commands to run
sagemath scripts to validate PUF authenticity.

#### Running the scipts in non-interactive mode

The non-interactive mode allows for verifying PUF authenticity by passing all
necessary variables as sys-args. Below is an example of non-interactive command
execution:

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

_Note: Nonce (the last param) is 64 bytes long._

Refer to "[obtaining values for the script](#obtaining-values-for-the-script)"
to learn how to obtain the values.

#### Running in interactive mode

Running the script in interactive mode will prompt user for inputs during script
execution. The command is as follows:

```bash
docker run -it -v $(pwd)/puf_vm1/scripts/proofs/:/mnt sagemath/sagemath sage /mnt/proof_verifier_calc.sage -i
```

#### Obtaining values for the script

If debug mode is enabled, the PUF VM will print PUF responses during the
execution, these look as follows:

```log
MEMREF Output data:
f1 52 1b d1 2c 23 fb ba  0c 96 1b 78 3f ca a2 02 |.R..,#.. ...x?...
5c 6a bf b1 1c b3 e2 70  0d ba 7f 83 1c 18 39 bb |\j.....p ......9.
Param[1] type: MEMREF_INOUT
MEMREF Output data:
32 d3 b9 62 32 b1 40 93  21 8f 02 d5 2a fd cc 60 |2..b2.@. !...*..`
2b ef f1 ae 59 e8 38 ff  f1 b4 a8 d6 a1 18 de df |+...Y.8. ........
```

These are the values needed for the scripts. There should be 10 blocks like this
printed in total. The single block is `MEMREF Output data:` header followed
by 32 or 64 byte hex array. These are the values in exact order the script
excepts. The exception to that rule is the last (`-n` - `nonce`) param, which
is a constant, predefined value that comes
[from tls-server source code](https://git.3mdeb.com/3mdeb/crosscon-uc1-2/src/commit/5c429a126d25c22dc9d9ce95838265e7640324ac/include/local_challenge.c#L32).

**FIX ME: Update the link to the file when PR is merged.**

#### Generating the command

Converting the hex tables that are printed on output is tedious, and most of
these values change every execution. The free versions of Chat-GPT are no good
for converting these either.

To get the command the least painful way, one can use the
`resources/scripts/cnvrt_puf_output.py`. The script expect an input in form of
a file containing all ten blocks described in
[the previous section](#obtaining-values-for-the-script). An example input
file can be found in the same directory. The script outputs a command
that can be directly executed to verify PUF authenticity.

```bash
python3 resources/scripts/cnvrt_puf_output.py resources/scripts/cnvrt_puf_output_sample_input.txt
```
