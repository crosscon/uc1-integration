#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(MAIN);
#include <stdio.h>
#include "crosscon_hv_config.h"
#include <zephyr/kernel.h>
#include <string.h>

#define TEE_UUID_LEN 16

void clear_mem(void)
{
    memset(message[0], 0, MESSAGE0_SIZE);
    memset(message[1], 0, MESSAGE1_SIZE);
    memset(message[2], 0, MESSAGE2_SIZE);
    memset(message[3], 0, MESSAGE3_SIZE);
    memset(message[4], 0, MESSAGE4_SIZE);
    memset(message[5], 0, MESSAGE5_SIZE);
    memset(message[6], 0, MESSAGE6_SIZE);
    memset(message[7], 0, MESSAGE7_SIZE);
    memset(message[8], 0, MESSAGE8_SIZE);
    memset(message[9], 0, MESSAGE9_SIZE);
    memset(message[10], 0, MESSAGE10_SIZE);
    memset(message[11], 0, MESSAGE11_SIZE);
    memset(message[12], 0, MESSAGE12_SIZE);
    memset(message[13], 0, MESSAGE13_SIZE);
}

void vm_init() {
    IRQ_CONNECT(IPC_IRQ_ID, 0, ipc_irq_handler, NULL, 0);
    irq_enable(IPC_IRQ_ID);
    clear_mem();
}

int call_puf(void)
{
    static const uint8_t uuid0[TEE_UUID_LEN] = {
        0x00, 0x11, 0x22, 0x33, /* timeLow    */
        0x44, 0x55,             /* timeMid    */
        0x66, 0x77,             /* timeHi+ver */
        0x88, 0x99, 0xAA, 0xBB, /* clockSeq   */
        0xCC, 0xDD, 0xEE, 0xFF  /* node       */
    };

    vm_init();

    LOG_INF("Calling func 1");
    memcpy((void*)message[0], &uuid0, sizeof(uuid0));
    ipc_notify(0,0);

    k_msleep(500);
    clear_mem();

    // Wait for interrupts and handle them according to function_table
    while(1);
}
