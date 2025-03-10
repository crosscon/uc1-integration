#include <config.h>

struct config config = {
    .vmlist_size = 2,
    .vmlist = (struct vm_config[]) {
        {
            .image = VM_IMAGE_LOADED(0x00020000, 0x00020000, 0xD00),

            .entry = 0x00020000,

            .platform = {
                .cpu_num = 1,
                .region_num = 2,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0x20010000, //SRAM1
                        .size = 0x8000
                    },
                    {
                        .base = 0x00020000,
                        .size = 0x10000
                    }
                },

                .dev_num = 1,
                .devs =  (struct vm_dev_region[]) {
                    {
                        /* Flexcomm Interface 1 (USART1) */
                        .pa = 0x40087000,
                        .va = 0x40087000,
                        .size = 0x1000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {15+15}
                    },
                },
            },
        },
        {
            .image = VM_IMAGE_LOADED(0x00040000, 0x00040000, 0xD00),

            .entry = 0x00040000,

            .platform = {
                .cpu_num = 1,
                .region_num = 2,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0x20030000, //SRAM1
                        .size = 0x8000
                    },
                    {
                        .base = 0x00040000,
                        .size = 0x10000
                    }
                },
            },
        },
    },
};