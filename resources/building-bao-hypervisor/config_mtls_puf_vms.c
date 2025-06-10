#include <config.h>

struct vm_config zephyr_vm0 = {
    // FIX ME
    // .image= was removed, it's redundant at the moment.
    .entry = @@ZEPHYR_VM0_ENTRY@@,
    .platform = {
        .cpu_num = 1,
        .region_num = 2,
        .regions =  (struct vm_mem_region[]) {
            {
                .base = 0x20030000, /* As in dtsi */
                .size = 0x10000 /* 64 KB */
            },
            {
                .base = 0x00048000,
                .size = 0x40000
            }
        },
        .dev_num = 4,
        .devs =  (struct vm_dev_region[]) {
            {
                /* Flexcomm Interface 2 (USART2) + USART3 */
                .pa = 0x40088000,
                .va = 0x40088000,
                .size = 0x2000,
                .interrupt_num = 2,
                .interrupts = (irqid_t[]) {16+16, 16+17}
            },
            {
                /* SYSCON + IOCON + PINT + SPINT */
                .pa = 0x40000000,
                .va = 0x40000000,
                .size = 0x5000,
            },
            {
                /* ANALOG + POWER MGM  */
                .pa = 0x40013000,
                .va = 0x40013000,
                .size = 0xE000,
            },
            {
                /* RNG */
                .pa = 0x4003a000,
                .va = 0x4003a000,
                .size = 0x1000,
            },
        },
        .ipc_num = 1,
        .ipcs = (struct ipc[]) {
            {
                .base = 0x20027000,
                .size = 0x1000,
                .shmem_id = 0,
                .interrupt_num = 1,
                .interrupts = (irqid_t[]) {79}
            }
        },
    }
};

struct vm_config zephyr_vm1 = {
    // FIX ME
    // .image= was removed, it's redundant at the moment.
    .entry = @@ZEPHYR_VM1_ENTRY@@,
    .platform = {
        .cpu_num = 1,
        .region_num = 2,
        .regions =  (struct vm_mem_region[]) {
            {
                .base = 0x20010000, /* As in overlay */
                .size = 0x10000 /* 64 KB*/
            },
            {
                .base = 0x00020000,
                .size = 0x28000
            }
        },
        .dev_num = 2,
        .devs =  (struct vm_dev_region[]) {
            {
                /* SYSCON + IOCON + PINT + SPINT */
                .pa = 0x40000000,
                .va = 0x40000000,
                .size = 0x5000,
            },
            {
                /* ANALOG + POWER MGM  */
                .pa = 0x40013000,
                .va = 0x40013000,
                .size = 0xE000,
            },
        },
        .ipc_num = 1,
        .ipcs = (struct ipc[]) {
            {
                .base = 0x20027000,
                .size = 0x1000,
                .shmem_id = 0,
                .interrupt_num = 1,
                .interrupts = (irqid_t[]) {78}
            }
        },
    }
};

struct config config = {

    CONFIG_HEADER
    .shmemlist_size = 1,
    .shmemlist = (struct shmem[]) {
        [0] = {.base = 0x20027000, .size = 0x1000,},
    },
    .vmlist_size = 2,
    .vmlist = (struct vm_config*[]) {
        &zephyr_vm0,
        &zephyr_vm1
    }
};
