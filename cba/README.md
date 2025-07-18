# Context-Based Authentication

This readme provides steps to reproduce the CBA demo that should be used during
the development of UC1.2.

## Building remote server

Follow the steps below to launch the remote server:

1. Clone and checkout the [context-based-auth-remote
  repository](https://github.com/crosscon/context-based-auth-remote):

    ```bash
    git clone https://github.com/crosscon/context-based-auth-remote.git
    cd context-based-auth-remote
    git checkout 45338c7221089058ea09d93f9c17538e118b0aa3
    ```

3. Build the container:

    ```bash
    docker build -t crosscon-ra-remote:latest .
    ```

4. Run the server:

    ```bash
    docker compose -f ./docker-compose.yml up
    ```

5. Collect the test signature that [will be
  needed](https://github.com/crosscon/context-based-auth-crosscon-demo/tree/3594d0b029f57422d00420e64369f199981d7e75?tab=readme-ov-file#configuration)
  for CBA TA host application:

    ```bash
    python ./demo_signature.py
    char SERVER_TEST_SIGNATURE[71] = { 48, 69, 2, 32, 4, 247, 113, 13, 200, 32, 82, 202, 123, 23, 119, 156, 212, 183, 197, 41, 241, 59, 222, 195, 240, 7, 93, 115, 111, 224, 238, 74, 4, 116, 101, 2, 2, 33, 0, 209, 50, 17, 198, 203, 73, 253, 171, 74, 30, 173, 126, 76, 126, 253, 15, 41, 239, 10, 184, 61, 24, 230, 6, 243, 171, 94, 227, 186, 223, 31, 23 };
    ```

    > Note: this is an example signature, you need to generate your own.

6. Check the IP address of the machine the server is running on by using, for
  example, `ip a` command. This IP [will be
  needed](https://github.com/crosscon/context-based-auth-crosscon-demo/tree/3594d0b029f57422d00420e64369f199981d7e75?tab=readme-ov-file#configuration)
  for CBA Pseudo TA.

## Building RPi image

Follow the steps below to build the RPi image:

1. Clone the [`context-based-auth-crosscon-demo`
repository](https://github.com/crosscon/context-based-auth-crosscon-demo) and
checkout the needed commit:

    ```bash
    git clone https://github.com/crosscon/context-based-auth-crosscon-demo.git
    cd context-based-auth-crosscon-demo
    git checkout 3594d0b029f57422d00420e64369f199981d7e75
    git submodule update --init --recursive
    ```

2. [Build and
enter](https://github.com/crosscon/context-based-auth-crosscon-demo/tree/3594d0b029f57422d00420e64369f199981d7e75/env#build--run-the-container)
  the container.
3. Copy [devicetree
  `rpi4-ws/nexmon.dts`](https://github.com/crosscon/context-based-auth-nexmon-vm/blob/9ad70a6ef05947c99b8dcfb205d7bca6f7813eb0/rpi4-ws/nexmon.dts) to the `rpi4-ws` directory and prepare
  Nexmon VM image (execute these commands inside the container):

    ```bash
    dtc -I dts -O dtb -o rpi4-ws/nexmon.dts rpi4-ws/nexmon.dtb
    cd lloader
    rm ./nexmon.bin
    rm ./nexmon.elf
    make  \
        IMAGE=../nexmon/nexmon-image \
        DTB=../rpi4-ws/nexmon.dtb \
        TARGET=nexmon.bin \
        CROSS_COMPILE=aarch64-none-elf- \
        ARCH=aarch64

    cp ./nexmon.bin ../nexmon/nexmon.bin
    cd $ROOT
    ```

4. [Configure](https://github.com/crosscon/context-based-auth-crosscon-demo/tree/3594d0b029f57422d00420e64369f199981d7e75?tab=readme-ov-file#configuration)
  the CBA TA, CBA Pseudo TA and CBA Host Application. The following changes
  worked when writing this readme:

    ```diff
    diff --git a/host/main.c b/host/main.c
    index 24e07dc5884b..c6c823139c28 100644
    --- a/host/main.c
    +++ b/host/main.c
    @@ -179,7 +179,7 @@ void test_verify() {
     
         char nonce_buffer[16];
         memset(nonce_buffer, 0, sizeof(nonce_buffer));
    -    char signature_buffer[71] = { 48, 69, 2, 33, 0, 238, 79, 112, 36, 34, 39, 135, 111, 5, 163, 245, 18, 25, 141, 101, 208, 126, 207, 17, 186, 27, 110, 168, 119, 161, 30, 50, 57, 93, 94, 164, 210, 2, 32, 21, 27, 55, 25, 232, 5, 147, 139, 92, 113, 13, 15, 178, 212, 240, 147, 20, 202, 89, 124, 194, 185, 234, 228, 2, 3, 98, 70, 57, 122, 147, 21 };
    +    char signature_buffer[70] = { 48, 68, 2, 32, 126, 89, 96, 202, 39, 181, 78, 26, 202, 252, 60, 155, 115, 184, 216, 221, 64, 99, 63, 151, 245, 236, 104, 178, 156, 155, 78, 189, 17, 132, 124, 175, 2, 32, 4, 35, 19, 229, 221, 234, 221, 47, 188, 102, 47, 121, 191, 13, 167, 90, 91, 187, 26, 214, 137, 131, 129, 26, 188, 243, 206, 237, 78, 18, 199, 115 };
     
         op.params[0].tmpref.buffer = nonce_buffer;
         op.params[0].tmpref.size = sizeof(nonce_buffer);
    diff --git a/ta/cba.c b/ta/cba.c
    index 852c60ec57fe..7caca5b5a023 100644
    --- a/ta/cba.c
    +++ b/ta/cba.c
    @@ -15,8 +15,8 @@
     
     
     #define TA_CONTEXT_BASED_AUTHENTICATION_BANDWIDTH               20      // either 20, 40, or 80 MHz
    -#define TA_CONTEXT_BASED_AUTHENTICATION_WIFI_CHANNEL            11      // must be a valid WiFi channel
    -#define TA_CONTEXT_BASED_AUTHENTICATION_RECORDING_TIMEOUT       60
    +#define TA_CONTEXT_BASED_AUTHENTICATION_WIFI_CHANNEL            1      // must be a valid WiFi channel
    +#define TA_CONTEXT_BASED_AUTHENTICATION_RECORDING_TIMEOUT       180
     #define TA_CONTEXT_BASED_AUTHENTICATION_SAMPLES_PER_DEVICE      128
     
     
    diff --git a/ta/network_handling.c b/ta/network_handling.c
    index 5d96d4936968..876b1f98db65 100644
    --- a/ta/network_handling.c
    +++ b/ta/network_handling.c
    @@ -14,7 +14,7 @@
     #include "tee_api_defines.h"
     
     
    -#define CONTEXT_BASED_AUTHENTICATION_SERVER_HOST   "192.168.42.1"
    +#define CONTEXT_BASED_AUTHENTICATION_SERVER_HOST   "192.168.10.51"
     #define CONTEXT_BASED_AUTHENTICATION_SERVER_PORT   5432
     
     const char* CONTEXT_BASED_AUTHENTICATION_SERVER_SSL_CERT = "-----BEGIN CERTIFICATE-----\n"

    ```

    There was no need to configure the CBA Pseudo TA, which is located under
    `optee_os/core/pta/`.

    > Note: use the IP and `signature_buffer` from [the previous
    > chapter](#building-remote-server). The WiFi inf. can be acquired via, for
    > example, `sudo iw wlp0s20f3 scan | grep -E 'SSID:|primary channel|width'`
    > command.

4. [Build](https://github.com/crosscon/context-based-auth-crosscon-demo/tree/3594d0b029f57422d00420e64369f199981d7e75/env#building-the-rpi4-ws-demo)
  and [prepare](https://github.com/crosscon/context-based-auth-crosscon-demo/tree/3594d0b029f57422d00420e64369f199981d7e75/env#creating-and-flashing-the-image)
  the image.
5. [Run](https://github.com/crosscon/context-based-auth-crosscon-demo/tree/3594d0b029f57422d00420e64369f199981d7e75/env#running-the-image)
  the image.

## Verifying the demo

Follow the steps below to verify the demo works correctly:

1. Launch hypervisor on RPi:

    ```bash
    fatload mmc 0 0x200000 crossconhyp.bin; go 0x200000
    ```

2. During booting of the Linux image the, ACT LED (the green LED near the red PWR
  LED) should flick in two different ways. Firstly, it will flick "quick ON...
  quick OFF...quick ON...long OFF" several times. Then, it should turn on and
  off several times with 1-second intervals and power off. If both flick
  sequence finishes, it means that the Nexmon VM is UP and running. **Do not
  proceed with steps until you are sure it is up and running!**
3. Log into the Linux by typing `root`, connect the RJ45 to the RPi, and
  launch DHCP client:

    ```bash
    udhcpc -i eth0
    ```

4. After the RPi receives the IP, run [the testing
  commands](https://github.com/crosscon/context-based-auth-crosscon-demo/tree/3594d0b029f57422d00420e64369f199981d7e75?tab=readme-ov-file#testing). Do not run the `devmem` commands.

5. You should receive `TA result: Ok` for all commands, e.g.:

    ```bash
    # context_based_authentication_demo enroll
    TA result: Ok.
    # context_based_authentication_demo prove
    TA result: Ok
    # context_based_authentication_demo verify
    TA result: Ok
    ```

## Issues and debugging tips

### RPi image

#### Kernel panics

The following Linux kernel panics:

```bash
[    1.439634] ------------[ cut here ]------------
[    1.439655] clk_core_populate_parent_map: invalid NULL in dsi0p's .parent_names
[    1.439709] WARNING: CPU: 0 PID: 1 at drivers/clk/clk.c:3745 __clk_register+0x450/0x820
[    1.439756] Modules linked in:
[    1.439777] CPU: 0 PID: 1 Comm: swapper/0 Not tainted 5.11.0-g0eb9b5b5f26d #11
[    1.439802] Hardware name: Raspberry Pi 4 Model B (DT)
[    1.439817] pstate: 60000005 (nZCv daif -PAN -UAO -TCO BTYPE=--)
[    1.439839] pc : __clk_register+0x450/0x820
[    1.439861] lr : __clk_register+0x450/0x820
[    1.439882] sp : ffffffc0100139d0
[    1.439895] x29: ffffffc0100139d0 x28: 000000000000000a 
[    1.439922] x27: ffffffc010013b08 x26: 0000000000000100 
[    1.439948] x25: ffffffc0115a1578 x24: ffffff80270bc400 
[    1.439972] x23: 0000000000000008 x22: ffffff80270bc540 
[    1.439996] x21: ffffff80270bd080 x20: 0000000000000008 
[    1.440020] x19: ffffff80270bd200 x18: ffffffffffffffff 
[    1.440043] x17: 0000000000000068 x16: 0000000000000001 
[    1.440066] x15: ffffffc090013697 x14: 0720072007200720 
[    1.440090] x13: 0720072007200720 x12: 0720072007200720 
[    1.440113] x11: ffffffc012d527e0 x10: ffffffc012d527e0 
[    1.440136] x9 : 00000000ffffefff x8 : ffffffc012daa7e0 
[    1.440160] x7 : 0000000000017fe8 x6 : 00000000fffff000 
[    1.440184] x5 : 0000000000000000 x4 : 0000000000000000 
[    1.440207] x3 : 00000000ffffffff x2 : 0000000000000000 
[    1.440230] x1 : 0000000000000000 x0 : ffffff8023450000 
[    1.440253] Call trace:
[    1.440267]  __clk_register+0x450/0x820
[    1.440289]  devm_clk_hw_register+0x54/0xd0
[    1.440312]  bcm2835_register_clock+0x10c/0x210
[    1.440332]  bcm2835_clk_probe+0xf4/0x1e0
[    1.440350]  platform_probe+0x68/0xe0
[    1.440373]  really_probe+0xe4/0x4c0
[    1.440391]  driver_probe_device+0x58/0xc0
[    1.440411]  device_driver_attach+0xc0/0xd0
[    1.440430]  __driver_attach+0x84/0x130
[    1.440449]  bus_for_each_dev+0x70/0xd0
[    1.440467]  driver_attach+0x24/0x30
[    1.440485]  bus_add_driver+0x104/0x1f0
[    1.440503]  driver_register+0x78/0x130
[    1.440522]  __platform_driver_register+0x28/0x40
[    1.440544]  bcm2835_clk_driver_init+0x1c/0x28
[    1.440564]  do_one_initcall+0x50/0x1b0
[    1.440584]  kernel_init_freeable+0x1d4/0x23c
[    1.440604]  kernel_init+0x14/0x118
[    1.440622]  ret_from_fork+0x10/0x34
[    1.440645] ---[ end trace 6d5d6a38f5a82d3c ]---
[    1.444741] ------------[ cut here ]------------
[    1.444768] bcm2835-dma fe007000.dma: DMA addr 0xffffffffffffffff+4096 overflow (mask ffffffff, bus limit ffffffff.
[    1.444806] WARNING: CPU: 0 PID: 1 at kernel/dma/direct.h:97 dma_map_page_attrs+0x1ec/0x200
[    1.444849] Modules linked in:
[    1.444868] CPU: 0 PID: 1 Comm: swapper/0 Tainted: G        W         5.11.0-g0eb9b5b5f26d #11
[    1.444893] Hardware name: Raspberry Pi 4 Model B (DT)
[    1.444908] pstate: 60000005 (nZCv daif -PAN -UAO -TCO BTYPE=--)
[    1.444929] pc : dma_map_page_attrs+0x1ec/0x200
[    1.444950] lr : dma_map_page_attrs+0x1ec/0x200
[    1.444970] sp : ffffffc010013af0
[    1.444982] x29: ffffffc010013af0 x28: 0000000000000000 
[    1.445008] x27: ffffff8027085080 x26: ffffffc012f920f0 
[    1.445032] x25: ffffffc01164b8d8 x24: 0000000000000000 
[    1.445056] x23: ffffffc012f541e8 x22: ffffff802358ac10 
[    1.445080] x21: ffffff802358ac00 x20: 0000000000001000 
[    1.445103] x19: ffffff802358ac10 x18: ffffffffffffffff 
[    1.445126] x17: 0000000000000001 x16: 0000000000000002 
[    1.445149] x15: ffffffc0900137b7 x14: 0720072007200720 
[    1.445172] x13: 0720072007200720 x12: 0720072007200720 
[    1.445196] x11: ffffffc012d527e0 x10: ffffffc012d527e0 
[    1.445218] x9 : 00000000ffffefff x8 : ffffffc012daa7e0 
[    1.445242] x7 : 0000000000017fe8 x6 : 00000000fffff000 
[    1.445266] x5 : 0000000000000000 x4 : 0000000000000000 
[    1.445288] x3 : 00000000ffffffff x2 : 0000000000000000 
[    1.445311] x1 : 0000000000000000 x0 : ffffff8023450000 
[    1.445335] Call trace:
[    1.445347]  dma_map_page_attrs+0x1ec/0x200
[    1.445368]  bcm2835_dma_probe+0x214/0x500
[    1.445395]  platform_probe+0x68/0xe0
[    1.445418]  really_probe+0xe4/0x4c0
[    1.445437]  driver_probe_device+0x58/0xc0
[    1.445456]  device_driver_attach+0xc0/0xd0
[    1.445475]  __driver_attach+0x84/0x130
[    1.445494]  bus_for_each_dev+0x70/0xd0
[    1.445511]  driver_attach+0x24/0x30
[    1.445529]  bus_add_driver+0x104/0x1f0
[    1.445547]  driver_register+0x78/0x130
[    1.445566]  __platform_driver_register+0x28/0x40
[    1.445588]  bcm2835_dma_driver_init+0x1c/0x28
[    1.445609]  do_one_initcall+0x50/0x1b0
[    1.445627]  kernel_init_freeable+0x1d4/0x23c
[    1.445646]  kernel_init+0x14/0x118
[    1.445664]  ret_from_fork+0x10/0x34
[    1.445683] ---[ end trace 6d5d6a38f5a82d3d ]---
[    1.445701] bcm2835-dma fe007000.dma: Failed to map zero page
[    1.445737] bcm2835-dma: probe of fe007000.dma failed with error -12
[    1.453947] Serial: 8250/16550 driver, 4 ports, IRQ sharing enabled
[    1.456090] fe215040.serial: ttyS1 at MMIO 0xfe215040 (irq = 15, base_baud = 62499999) is a 16550
```

Are expected.

#### Nexmon VM compilation issues

The Nexmon VM could be compiled by following [the
README](https://github.com/crosscon/context-based-auth-nexmon-vm/blob/kernel_5.10.92-v8%2B_nexmon_automated/README.md)
from the commit `9ad70a6ef05947c99b8dcfb205d7bca6f7813eb0`. Unfortunately, due
to some unknown issues, it was decided to use a pre-compiled Nexmon VM. The
binary is being prepared in step 3 in the [Building RPi
image](#building-rpi-image) chapter.

### Remote server

Add `PYTHONUNBUFFERED=1` to the `.env` file in the `context-based-auth-remote`
repository to make the remote server print logs. Use the following command to
run the remote server's Docker container in interactive mode:

```bash
docker run --rm -it --volume './keys:/app/keys:ro' --volume './verified:/verified:ro' -p '5432:5432' --env-file '.env' 'inference-server:cba' /bin/bash
```
