#include <config.h>

struct vm_config zephyr = {
    .image = VM_IMAGE_LOADED(0x00040000, 0x00040000, 0xF000),
    .entry = @@ZEPHYR_VM_ENTRY@@,
    .platform = {
        .cpu_num = 1,
        .region_num = 2,
        .regions =  (struct vm_mem_region[]) {
            {
                .base = 0x20020000, //SRAM1
                .size = 0x10000
            },
            {
                .base = 0x00040000,
                .size = 0x18000
            }
        },
        .dev_num = 4,
        .devs =  (struct vm_dev_region[]) {
            {
                /* Flexcomm Interface 2 (USART2) */
                /* AND */
                /* Flexcomm Interface 3 (USART3) */
                .pa = 0x40088000,
                .va = 0x40088000,
                .size = 0x2000,
                .interrupt_num = 2,
                .interrupts = (irqid_t[]) {16+16, 17+16}
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
                .base = 0x20017000,
                .size = 0x1000,
                .shmem_id = 0,
                .interrupt_num = 1,
                .interrupts = (irqid_t[]) {79}
            }
        },
    }
};

struct config config = {

    CONFIG_HEADER
    .shmemlist_size = 1,
    .shmemlist = (struct shmem[]) {
        [0] = {.base = 0x20017000, .size = 0x1000,},
    },
    .vmlist_size = 1,
    .vmlist = (struct vm_config*[]) {
        &zephyr
    }
};
