# Building wolfSSL benchmark sample

This guide goes over how to build the wolfSSL benchmark sample. It  is assumed
that the user has a properly configured `west` and `zephyr` environment. In
order to achieve that, use the official zephyr documentation:

- [follow the getting started guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)
to get the development environment
- [build the hello world sample](https://docs.zephyrproject.org/latest/samples/hello_world/README.html)
to make sure that everything works
- [build for the lpcxpresso55s69 board](https://docs.zephyrproject.org/latest/boards/nxp/lpcxpresso55s69/doc/index.html)
(`cpu0` target)

## Adding wolfSSL module to build

Edit the `west.yml` file, and add this:

```yml
    - name: wolfssl
      url-base: https://github.com/wolfssl
```

to the `manifest.remotes` section. Then add this:

```yml
    - name: wolfssl
      path: modules/crypto/wolfssl
      revision: master
      remote: wolfssl
```

to the `manifest.projects` section. Then run `west update`, and the new module
should appear:

```bash
user in ~/zephyr:# ls ../modules/crypto
mbedtls  tinycrypt  wolfssl
user in ~/zephyr:#
```

## Modifying the build so that it works

Straight out of the box, the build doesn't work. Two edits to the config files
in the `benchmark` sample of the `wolfSSL` module have to be made.

Remember, the `benchmark` test files are located in the module, so if you are
in the `zephyr` directory, they are one level above, like this:

```bash
wgrzywacz in ~/Desktop/work/zephyr-stuff/zephyr on main ● ● λ ls ../modules/crypto/wolfssl/zephyr/samples
wolfssl_benchmark  wolfssl_test  wolfssl_tls_sock  wolfssl_tls_thread
wgrzywacz in ~/Desktop/work/zephyr-stuff/zephyr on main ● ● λ
```

> Note: this was tested with `main` branch, commit
> `3ca444e0e126190dcfbaf522037b09f09e47463d`, as earlier tags were causing
> problems.

```patch
diff --git a/zephyr/samples/wolfssl_benchmark/prj.conf b/zephyr/samples/wolfssl_benchmark/prj.conf
index 017988024946..ae994eb92033 100644
--- a/zephyr/samples/wolfssl_benchmark/prj.conf
+++ b/zephyr/samples/wolfssl_benchmark/prj.conf
@@ -3,10 +3,13 @@ CONFIG_MAIN_STACK_SIZE=32768
 CONFIG_COMMON_LIBC_MALLOC_ARENA_SIZE=8192

 # Pthreads
-CONFIG_PTHREAD_IPC=y
+CONFIG_POSIX_THREADS=y

 # Clock for time()
-CONFIG_POSIX_CLOCK=y
+CONFIG_POSIX_API=y
+CONFIG_POSIX_TIMERS=y
+
+CONFIG_POSIX_C_LIB_EXT=y

 # TLS configuration
 CONFIG_WOLFSSL=y
```

```patch
diff --git a/zephyr/samples/wolfssl_benchmark/CMakeLists.txt b/zephyr/samples/wolfssl_benchmark/CMakeLists.txt
index 1b68e1bfb7c6..91ef08ccdd28 100644
--- a/zephyr/samples/wolfssl_benchmark/CMakeLists.txt
+++ b/zephyr/samples/wolfssl_benchmark/CMakeLists.txt
@@ -2,6 +2,8 @@ cmake_minimum_required(VERSION 3.13.1)
 find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
 project(wolfssl_benchmark)

+add_definitions(-DWOLFSSL_USER_SETTINGS=\"user_settings.h\")
+
 target_sources(app PRIVATE ${ZEPHYR_WOLFSSL_MODULE_DIR}/wolfcrypt/benchmark/benchmark.c)
 target_include_directories(app PRIVATE ${ZEPHYR_WOLFSSL_MODULE_DIR}/wolfcrypt/benchmark)
 target_sources(app PRIVATE ${app_sources})
```

then, at this location `modules/crypto/wolfssl/user_settings.h` make a file:

```c
#ifndef USER_SETTINGS_H
#define USER_SETTINGS_H

#define NO_OLD_RNGNAME
#define WOLFSSL_NO_FILESYSTEM
#define HAVE_TLS_EXTENSIONS
#define WOLFSSL_TLS13
#define WOLFSSL_TLS13_CLIENT
#define WOLFSSL_TLS13_SERVER
#define HAVE_WOLFSSL_TEST
#define HAVE_FFDHE_2048
#define WC_RSA_PSS
#define HAVE_AEAD
#define HAVE_HKDF
#define WOLFSSL_PTHREADS
#define HAVE_THREAD

#endif /* USER_SETTINGS_H */
```

## Building, flashing and logs

Then finally the build can be ran with this command:

```bash
west build -p auto -b lpcxpresso55s69/lpc55s69/cpu0 ../modules/crypto/wolfssl/zephyr/samples/wolfssl_benchmark
```

the binary ends up at `build/zephyr/zephyr.bin`, and after
[flashing](./lpcxpresso55s69-flashing-guide.md) and observing `minicom` it
gives this output:

```minicom
*** Booting Zephyr OS build v4.1.0-2563-g90cd350c5a4a ***
------------------------------------------------------------------------------
 wolfSSL version 5.7.6
------------------------------------------------------------------------------
wolfCrypt Benchmark (block bytes 1024, min 1.0 sec each)
RNG                        375 KiB took 1.000 seconds,  375.000 KiB/s
AES-128-CBC-enc            475 KiB took 1.000 seconds,  475.000 KiB/s
AES-128-CBC-dec            475 KiB took 1.000 seconds,  475.000 KiB/s
AES-192-CBC-enc            425 KiB took 1.000 seconds,  425.000 KiB/s
AES-192-CBC-dec            400 KiB took 1.000 seconds,  400.000 KiB/s
AES-256-CBC-enc            350 KiB took 1.000 seconds,  350.000 KiB/s
AES-256-CBC-dec            350 KiB took 1.000 seconds,  350.000 KiB/s
3DES                       100 KiB took 1.000 seconds,  100.000 KiB/s
MD5                          4 MiB took 1.000 seconds,    4.370 MiB/s
SHA                          2 MiB took 1.000 seconds,    2.271 MiB/s
SHA-256                      1 MiB took 1.000 seconds,    1.367 MiB/s
HMAC-MD5                     5 MiB took 1.000 seconds,    4.565 MiB/s
HMAC-SHA                     2 MiB took 1.000 seconds,    2.246 MiB/s
HMAC-SHA256                  1 MiB took 1.000 seconds,    1.367 MiB/s
PBKDF2                     192 bytes took 1.000 seconds,  192.000 bytes/s
RSA     2048   public        30 ops took 1.000 sec, avg 33.333 ms, 30.000 ops/sec
RSA     2048  private         2 ops took 3.000 sec, avg 1500.000 ms, 0.667 ops/sec
DH      2048  key gen         1 ops took 1.000 sec, avg 1000.000 ms, 1.000 ops/sec
DH      2048    agree         2 ops took 2.000 sec, avg 1000.000 ms, 1.000 ops/sec
Benchmark complete
```

## Trying to build `wolfssl_tls_sock` sample

When trying to buidl the `wolfssl_tls_sock` sample, we have to make the same
edits as in the `benchmark` sample. We already have the `user_settings.h` file,
so we just need to include it in the
`modules/crypto/wolfssl/zephyr/samples/wolfssl_tls_sock/CMakeLists.txt` file,
and also change the
`modules/crypto/wolfssl/zephyr/samples/wolfssl_tls_sock/prj.conf` file. Both
of these edits can be made similarly to how they were done above.

This will make sure we don't get any compilation errors, but the build still
won't succeed:

```bash
wgrzywacz in ~/Desktop/work/zephyr-stuff/zephyr on main ● ● λ west build -p auto -b lpcxpresso55s69/lpc55s69/cpu0 ../modules/crypto/wolfssl/zephyr/samples/wolfssl_tls_sock
[1/6] Linking C executable zephyr/zephyr_pre0.elf
FAILED: zephyr/zephyr_pre0.elf zephyr/zephyr_pre0.map /home/wgrzywacz/Desktop/work/zephyr-stuff/zephyr/build/zephyr/zephyr_pre0.map
: && /opt/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc  -gdwarf-4 zephyr/CMakeFiles/zephyr_pre0.dir/misc/empty_file.c.obj -o zephyr/zephyr_pre0.elf  zephyr/CMakeFiles/offsets.dir/./arch/arm/core/offsets/offsets.c.obj  -T  zephyr/linker_zephyr_pre0.cmd  -Wl,-Map=/home/wgrzywacz/Desktop/work/zephyr-stuff/zephyr/build/zephyr/zephyr_pre0.map  -Wl,--whole-archive  app/libapp.a  zephyr/libzephyr.a  zephyr/arch/common/libarch__common.a  zephyr/arch/arch/arm/core/libarch__arm__core.a  zephyr/arch/arch/arm/core/cortex_m/libarch__arm__core__cortex_m.a  zephyr/arch/arch/arm/core/cortex_m/cmse/libarch__arm__core__cortex_m__cmse.a  zephyr/arch/arch/arm/core/mpu/libarch__arm__core__mpu.a  zephyr/lib/libc/newlib/liblib__libc__newlib.a  zephyr/lib/libc/common/liblib__libc__common.a  zephyr/lib/posix/options/liblib__posix__options.a  zephyr/lib/net_buf/liblib__net_buf.a  zephyr/lib/os/zvfs/liblib__os__zvfs.a  zephyr/soc/soc/lpc55s69/lpc55xxx/libsoc__nxp__lpc__lpc55xxx.a  zephyr/boards/nxp/lpcxpresso55s69/libboards__nxp__lpcxpresso55s69.a  zephyr/subsys/fs/libsubsys__fs.a  zephyr/subsys/random/libsubsys__random.a  zephyr/subsys/net/libsubsys__net.a  zephyr/subsys/net/l2/dummy/libsubsys__net__l2__dummy.a  zephyr/subsys/net/ip/libsubsys__net__ip.a  zephyr/subsys/net/lib/config/libsubsys__net__lib__config.a  zephyr/drivers/interrupt_controller/libdrivers__interrupt_controller.a  zephyr/drivers/clock_control/libdrivers__clock_control.a  zephyr/drivers/console/libdrivers__console.a  zephyr/drivers/entropy/libdrivers__entropy.a  zephyr/drivers/gpio/libdrivers__gpio.a  zephyr/drivers/pinctrl/libdrivers__pinctrl.a  zephyr/drivers/serial/libdrivers__serial.a  zephyr/drivers/timer/libdrivers__timer.a  modules/wolfssl/lib..__modules__crypto__wolfssl__zephyr.a  modules/hal_nxp/libmodules__hal_nxp.a  -Wl,--no-whole-archive  zephyr/kernel/libkernel.a  -L/home/wgrzywacz/Desktop/work/zephyr-stuff/zephyr/build/zephyr  zephyr/arch/common/libisr_tables.a  -mcpu=cortex-m33  -mthumb  -mabi=aapcs  -mfp16-format=ieee  -fuse-ld=bfd  -Wl,--gc-sections  -Wl,--build-id=none  -Wl,--sort-common=descending  -Wl,--sort-section=alignment  -Wl,-u,_OffsetAbsSyms  -Wl,-u,_ConfigAbsSyms  -nostdlib  -static  -Wl,-X  -Wl,-N  -Wl,--orphan-handling=warn  -Wl,-no-pie  -L"/opt/zephyr-sdk/arm-zephyr-eabi/arm-zephyr-eabi"/lib/thumb/v8-m.main/nofp -L"/opt/zephyr-sdk/arm-zephyr-eabi/bin/../lib/gcc/arm-zephyr-eabi/12.2.0/thumb/v8-m.main/nofp" -lm -lc -lgcc -lc && cd /home/wgrzywacz/Desktop/work/zephyr-stuff/zephyr/build/zephyr && /usr/bin/cmake -E true
/opt/zephyr-sdk/arm-zephyr-eabi/bin/../lib/gcc/arm-zephyr-eabi/12.2.0/../../../../arm-zephyr-eabi/bin/ld.bfd: zephyr/zephyr_pre0.elf section `text' will not fit in region `FLASH'
/opt/zephyr-sdk/arm-zephyr-eabi/bin/../lib/gcc/arm-zephyr-eabi/12.2.0/../../../../arm-zephyr-eabi/bin/ld.bfd: region `FLASH' overflowed by 57460 bytes
collect2: error: ld returned 1 exit status
ninja: build stopped: subcommand failed.
FATAL ERROR: command exited with status 1: /usr/bin/cmake --build /home/wgrzywacz/Desktop/work/zephyr-stuff/zephyr/build
wgrzywacz in ~/Desktop/work/zephyr-stuff/zephyr on main ● ● λ
```

from these lines:

```bash
zephyr/zephyr_pre0.elf section `text' will not fit in region `FLASH'
region `FLASH' overflowed by 57460 bytes
```

we can see that the binary is simply too big for the board. This issue is only
worse for the `cpu1` target of the same board. The only real fix I saw at the
time was trying to shrink the binary, by getting rid of things we don't need.

After some research, I managed to come up with this approach:

First, make a config file in the directory of the sample, so here:

```bash
modules/crypto/wolfssl/zephyr/samples/wolfssl_tls_sock/overlay.conf
```

that file will contain the binary-shrinking config:

```Kconfig
# generic size savers
CONFIG_SIZE_OPTIMIZATIONS=y      # -Os
CONFIG_ASSERT=n
CONFIG_DEBUG=n
CONFIG_SHELL=n
CONFIG_LOG=y
CONFIG_LOG_MODE_DEFERRED=y
CONFIG_LOG_DEFAULT_LEVEL=0
CONFIG_PRINTK=y

# link time optimisations
CONFIG_LTO=y
CONFIG_ISR_TABLES_LOCAL_DECLARATION=y

# C library
CONFIG_NEWLIB_LIBC=n
CONFIG_MINIMAL_LIBC=y

# networking
CONFIG_NET_IPV6=n
CONFIG_NET_UDP=n
CONFIG_NET_TCP=y
CONFIG_NET_MGMT=n
CONFIG_NET_PKT_RX_COUNT=2
CONFIG_NET_PKT_TX_COUNT=2

# wolfSSL build options
CONFIG_WOLFSSL_TLS_VERSION_1_3=n
CONFIG_WOLFSSL_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED=y
CONFIG_WOLFSSL_KEY_EXCHANGE_RSA_ENABLED=n
CONFIG_WOLFSSL_KEY_EXCHANGE_RSA_PSK_ENABLED=n
CONFIG_WOLFSSL_KEY_EXCHANGE_DHE_RSA_ENABLED=n
CONFIG_WOLFSSL_KEY_EXCHANGE_ECDHE_RSA_ENABLED=n
CONFIG_WOLFSSL_KEY_EXCHANGE_ECDHE_PSK_ENABLED=n
CONFIG_WOLFSSL_KEY_EXCHANGE_ECDH_ECDSA_ENABLED=n
CONFIG_WOLFSSL_KEY_EXCHANGE_ECDH_RSA_ENABLED=n
CONFIG_WOLFSSL_KEY_EXCHANGE_ECJPAKE_ENABLED=n

# curves
CONFIG_WOLFSSL_ECP_ALL_ENABLED=n
CONFIG_WOLFSSL_ECP_DP_SECP256R1_ENABLED=y

# symmetric crypto
CONFIG_WOLFSSL_CIPHER_ALL_ENABLED=n
CONFIG_WOLFSSL_CIPHER_AES_ENABLED=y
CONFIG_WOLFSSL_CIPHER_MODE_GCM_ENABLED=y

# hashes / MACs
CONFIG_WOLFSSL_MAC_ALL_ENABLED=n
CONFIG_WOLFSSL_MAC_SHA256_ENABLED=y

# misc
CONFIG_WOLFSSL_HAVE_ASM=n
CONFIG_WOLFSSL_AES_ROM_TABLES=n
```

It tries to get rid of anything that is not used in the sample. I made this
config file based on snippets I found online, that I thought were relevant and
could be used.

We tell the build system to include our extra config file and apply it at build
time, by invoking the build with this command:

```bash
west build -p auto -b lpcxpresso55s69/lpc55s69/cpu0 ../modules/crypto/wolfssl/zephyr/samples/wolfssl_tls_sock -- -DEXTRA_CONF_FILE=overlay.conf
```

Then, through our `-DEXTRA_CONF_FILE` our overlay gets applied.

However, this is still not enough to make the binary fit on the flash:

```bash
wgrzywacz in ~/Desktop/work/zephyr-stuff/zephyr on main ● ● λ west build -p auto -b lpcxpresso55s69/lpc55s69/cpu0 ../modules/crypto/wolfssl/zephyr/samples/wolfssl_tls_sock -- -DEXTRA_CONF_FILE=overlay_no_pm.conf
-- west build: generating a build system
# skipped some lines here for brevity
-- west build: building application
[3/8] Linking C executable zephyr/zephyr_pre0.elf
FAILED: zephyr/zephyr_pre0.elf zephyr/zephyr_pre0.map /home/wgrzywacz/Desktop/work/zephyr-stuff/zephyr/build/zephyr/zephyr_pre0.map
: && /opt/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc  -gdwarf-4 -flto=auto -fno-ipa-sra -ffunction-sections -fdata-sections zephyr/CMakeFiles/zephyr_pre0.dir/misc/empty_file.c.obj -o zephyr/zephyr_pre0.elf  zephyr/CMakeFiles/offsets.dir/./arch/arm/core/offsets/offsets.c.obj  -T  zephyr/linker_zephyr_pre0.cmd  -Wl,-Map=/home/wgrzywacz/Desktop/work/zephyr-stuff/zephyr/build/zephyr/zephyr_pre0.map  -Wl,--whole-archive  app/libapp.a  zephyr/libzephyr.a  zephyr/arch/common/libarch__common.a  zephyr/arch/arch/arm/core/libarch__arm__core.a  zephyr/arch/arch/arm/core/cortex_m/libarch__arm__core__cortex_m.a  zephyr/arch/arch/arm/core/cortex_m/cmse/libarch__arm__core__cortex_m__cmse.a  zephyr/arch/arch/arm/core/mpu/libarch__arm__core__mpu.a  zephyr/lib/libc/picolibc/liblib__libc__picolibc.a  zephyr/lib/libc/common/liblib__libc__common.a  zephyr/lib/posix/options/liblib__posix__options.a  zephyr/lib/net_buf/liblib__net_buf.a  zephyr/lib/os/zvfs/liblib__os__zvfs.a  zephyr/soc/soc/lpc55s69/lpc55xxx/libsoc__nxp__lpc__lpc55xxx.a  zephyr/boards/nxp/lpcxpresso55s69/libboards__nxp__lpcxpresso55s69.a  zephyr/subsys/fs/libsubsys__fs.a  zephyr/subsys/random/libsubsys__random.a  zephyr/subsys/net/libsubsys__net.a  zephyr/subsys/net/l2/dummy/libsubsys__net__l2__dummy.a  zephyr/subsys/net/ip/libsubsys__net__ip.a  zephyr/subsys/net/lib/config/libsubsys__net__lib__config.a  zephyr/drivers/interrupt_controller/libdrivers__interrupt_controller.a  zephyr/drivers/clock_control/libdrivers__clock_control.a  zephyr/drivers/console/libdrivers__console.a  zephyr/drivers/entropy/libdrivers__entropy.a  zephyr/drivers/gpio/libdrivers__gpio.a  zephyr/drivers/pinctrl/libdrivers__pinctrl.a  zephyr/drivers/serial/libdrivers__serial.a  zephyr/drivers/timer/libdrivers__timer.a  modules/wolfssl/lib..__modules__crypto__wolfssl__zephyr.a  modules/hal_nxp/libmodules__hal_nxp.a  -Wl,--no-whole-archive  zephyr/kernel/libkernel.a  -L/home/wgrzywacz/Desktop/work/zephyr-stuff/zephyr/build/zephyr  zephyr/arch/common/libisr_tables.a  -mcpu=cortex-m33  -mthumb  -mabi=aapcs  -mfp16-format=ieee  -mtp=soft  -fuse-ld=bfd  -Wl,--gc-sections  -Wl,--build-id=none  -Wl,--sort-common=descending  -Wl,--sort-section=alignment  -Wl,-u,_OffsetAbsSyms  -Wl,-u,_ConfigAbsSyms  -nostdlib  -static  -Wl,-X  -Wl,-N  -Wl,--orphan-handling=warn  -Wl,-no-pie  -specs=picolibc.specs  -DPICOLIBC_LONG_LONG_PRINTF_SCANF -L"/opt/zephyr-sdk/arm-zephyr-eabi/bin/../lib/gcc/arm-zephyr-eabi/12.2.0/thumb/v8-m.main/nofp" -lc -lgcc && cd /home/wgrzywacz/Desktop/work/zephyr-stuff/zephyr/build/zephyr && /usr/bin/cmake -E true
/opt/zephyr-sdk/arm-zephyr-eabi/bin/../lib/gcc/arm-zephyr-eabi/12.2.0/../../../../arm-zephyr-eabi/bin/ld.bfd: zephyr/zephyr_pre0.elf section `text' will not fit in region `FLASH'
/opt/zephyr-sdk/arm-zephyr-eabi/bin/../lib/gcc/arm-zephyr-eabi/12.2.0/../../../../arm-zephyr-eabi/bin/ld.bfd: region `FLASH' overflowed by 29448 bytes
collect2: error: ld returned 1 exit status
ninja: build stopped: subcommand failed.
FATAL ERROR: command exited with status 1: /usr/bin/cmake --build /home/wgrzywacz/Desktop/work/zephyr-stuff/zephyr/build
wgrzywacz in ~/Desktop/work/zephyr-stuff/zephyr on main ● ● λ
```

As we can see, our overlay worked - the overflow is now less, `29448` bytes too
much, instead of the earlier `57460`. However it is still too much. During
tinkering with the config I have managed to get it as low as around `27000`, but
I can't remember the exact configuration.

There is a possibility this config could further be tweaked, but I personally
have ran out of ideas as to what else could be removed.
