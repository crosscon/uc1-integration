#include <config.h>

struct config config = {
    /**
     * This defines an array of shared memory objects that may be associated
     * with inter-partition communication objects in the VM platform definition
     * below using the shared memory object ID, ie, its index in the list.
     */
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
                        .size = 0x7000
                    },
                    {
                        .base = 0x00020000,
                        .size = 0x10000
                    }
                },
                .dev_num = 2,
                .devs =  (struct vm_dev_region[]) {
                    {
                        /* Flexcomm Interface 2 (USART2) */
                        .pa = 0x40088000,
                        .va = 0x40088000,
                        .size = 0x1000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {16+16}
                    },
                    {
                        /* SYSCON + IOCON */
                        .pa = 0x40000000,
                        .va = 0x40000000,
                        .size = 0x2000,
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
                .dev_num = 2,
                .devs =  (struct vm_dev_region[]) {
                    {
                        /* Flexcomm Interface 3 (USART3) */
                        .pa = 0x40089000,
                        .va = 0x40089000,
                        .size = 0x1000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {17+16}
                    },
                    {
                        /* SYSCON + IOCON */
                        .pa = 0x40000000,
                        .va = 0x40000000,
                        .size = 0x2000,
                    },
                },
            },
        },
    },
};