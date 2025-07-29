#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(GUEST_VM, LOG_LEVEL_DBG);

#include <stdio.h>
#include "crosscon_hv/crosscon_hv_config.h"
#include <zephyr/kernel.h>
#include "../common/challenge.h"

#define CLIENT_UUID_BYTES { \
    0x10, 0x20, 0x30, 0x40, \
    0x50, 0x60, \
    0x70, 0x80, \
    0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, 0x00 \
}

#define PUF_TA_UUID_BYTES { \
    0x00, 0x11, 0x22, 0x33, \
    0x44, 0x55, \
    0x66, 0x77, \
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF \
}

#define PUF_TA_INIT_FUNC_ID            0x00112233
#define PUF_TA_GET_COMMITMENT_FUNC_ID  0x11223344
#define PUF_TA_GET_ZK_PROOFS_FUNC_ID   0x22334455

static const uint8_t PUF_TA_UUID[TEE_UUID_LEN] = PUF_TA_UUID_BYTES;

static const uint8_t CLIENT_UUID[TEE_UUID_LEN] = CLIENT_UUID_BYTES;

// Helper functions to make the calling function logic easier to read
int call_puf_ta_init(const struct device *tee_dev, int session_id, func_call_t * call)
{
    param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[0].a    = (uint64_t)(VMS_MEMREF0_OFFSET); // offsets
    param[0].b    = (uint64_t)32;                   // length: 32 bytes
    param[0].c    = (uint64_t)(uintptr_t)&ipc_shm;  // shared-memory ID (handle)

    param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[1].a    = (uint64_t)(VMS_MEMREF1_OFFSET); // offsets
    param[1].b    = (uint64_t)32;                   // length: 32 bytes
    param[1].c    = (uint64_t)(uintptr_t)&ipc_shm;  // shared-memory ID (handle)

    param[2].attr = TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[2].a    = (uint64_t)(VMS_MEMREF2_OFFSET); // offsets
    param[2].b    = (uint64_t)32;                   // length: 32 bytes
    param[2].c    = (uint64_t)(uintptr_t)&ipc_shm;  // shared-memory ID (handle)

    param[3].attr = TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[3].a    = (uint64_t)(VMS_MEMREF3_OFFSET); // offsets
    param[3].b    = (uint64_t)32;                   // length: 32 bytes
    param[3].c    = (uint64_t)(uintptr_t)&ipc_shm;  // shared-memory ID (handle)

    invoke_func_arg.func      = call->func;
    invoke_func_arg.session   = session_id;
    invoke_func_arg.cancel_id = 0;
    invoke_func_arg.ret       = 0;
    invoke_func_arg.ret_origin= 0;

    return tee_invoke_func(tee_dev, &invoke_func_arg, 4, &param);
}

int call_puf_ta_get_commitment(const struct device *tee_dev, int session_id, uint8_t *shm_ptr, func_call_t * call)
{
    param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].a    = (uint64_t)(VMS_MEMREF0_OFFSET); // offsets
    param[0].b    = (uint64_t)32;                   // length: 32 bytes
    param[0].c    = (uint64_t)(uintptr_t)&ipc_shm;  // shared-memory ID (handle)

    param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[1].a    = (uint64_t)(VMS_MEMREF1_OFFSET); // offsets
    param[1].b    = (uint64_t)32;                   // length: 32 bytes
    param[1].c    = (uint64_t)(uintptr_t)&ipc_shm;  // shared-memory ID (handle)

    param[2].attr = TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[2].a    = (uint64_t)(VMS_MEMREF2_OFFSET); // offsets
    param[2].b    = (uint64_t)32;                   // length: 32 bytes
    param[2].c    = (uint64_t)(uintptr_t)&ipc_shm;  // shared-memory ID (handle)

    param[3].attr = TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[3].a    = (uint64_t)(VMS_MEMREF3_OFFSET); // offsets
    param[3].b    = (uint64_t)32;                   // length: 32 bytes
    param[3].c    = (uint64_t)(uintptr_t)&ipc_shm;  // shared-memory ID (handle)

    invoke_func_arg.func      = call->func;
    invoke_func_arg.session   = session_id;
    invoke_func_arg.cancel_id = 0;
    invoke_func_arg.ret       = 0;
    invoke_func_arg.ret_origin= 0;

    memset(shm_ptr, 0, 1024);
    memcpy((void *)(shm_ptr + param[0].a), call->data_p[0].data, param[0].b);
    memcpy((void *)(shm_ptr + param[1].a), call->data_p[0].data, param[1].b);

    return tee_invoke_func(tee_dev, &invoke_func_arg, 4, &param);
}


int call_puf_ta_get_zk_proofs(const struct device *tee_dev, int session_id, uint8_t *shm_ptr, func_call_t * call)
{
    param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
    param[0].a    = (uint64_t)(VMS_MEMREF0_OFFSET);
    param[0].b    = (uint64_t)32;
    param[0].c    = (uint64_t)(uintptr_t)&ipc_shm;

    param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
    param[1].a    = (uint64_t)(VMS_MEMREF1_OFFSET);
    param[1].b    = (uint64_t)32;
    param[1].c    = (uint64_t)(uintptr_t)&ipc_shm;

    param[2].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
    param[2].a    = (uint64_t)(VMS_MEMREF2_OFFSET);
    param[2].b    = (uint64_t)64;
    param[2].c    = (uint64_t)(uintptr_t)&ipc_shm;

    param[3].attr = TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
    param[3].a    = (uint64_t)(VMS_MEMREF3_OFFSET);
    param[3].b    = (uint64_t)64;
    param[3].c    = (uint64_t)(uintptr_t)&ipc_shm;

    invoke_func_arg.func      = call->func;
    invoke_func_arg.session   = session_id;
    invoke_func_arg.cancel_id = 0;
    invoke_func_arg.ret       = 0;
    invoke_func_arg.ret_origin= 0;

    memset(shm_ptr, 0, 1024);
    memcpy((void *)(shm_ptr + param[0].a), call->data_p[0].data, param[0].b);
    memcpy((void *)(shm_ptr + param[1].a), call->data_p[1].data, param[1].b);
    memcpy((void *)(shm_ptr + param[2].a), call->data_p[2].data, param[2].b);

    return tee_invoke_func(tee_dev, &invoke_func_arg, 4, &param);
}

void vm_init() {
    IRQ_CONNECT(IPC_IRQ_ID, 0, ipc_irq_client_handler, NULL, 0);
    irq_enable(IPC_IRQ_ID);
    LOG_INF("VM Initialized");
}

void cpy_last_result(func_call_t * call)
{
    for (uint8_t i=0; i < DATA_PORTIONS; i++) {
        const volatile uint8_t *data = &msg->payload[param[i].a];

        memcpy(call->data_p[i].data, data, call->data_p[i].len);
    }
}

int challenge_puf(func_call_t * init, func_call_t * commitment, func_call_t * proofs)
{
    vm_init();

    LOG_INF("Initializing TEE");

    /* Init CROSSCON HV TEE */
    const struct device *tee_dev = device_get_binding("crosscon_hv_tee");
    if (tee_dev == NULL) {
        LOG_ERR("Failed to bind device 'crosscon_hv_tee'");
        return -1;
    }
    struct tee_version_info info;
    int res;
    res = tee_get_version(tee_dev, &info);
    if (res == 0) {
        LOG_INF("TEE version info:");
        LOG_INF("impl_id   = %u", info.impl_id);
        LOG_INF("impl_caps = 0x%08x", info.impl_caps);
        LOG_INF("gen_caps  = 0x%08x", info.gen_caps);
    } else {
        LOG_ERR("tee_get_version() failed: %d", res);
    }

    /* Point shared mem to the device */
    ipc_shm.dev   = tee_dev;                            // from device_get_binding()
    ipc_shm.addr  = (void *)(uintptr_t)VMS_PAYLOAD_PTR; // static shared memory base
    ipc_shm.size  = (uint64_t)VMS_USABLE_PAYLOAD_SIZE;  // total available size
    ipc_shm.flags = 0;                                  // no special flags

    /* Buffer to write input to */
    volatile uint8_t *shm_ptr = (volatile uint8_t *)ipc_shm.addr;

    LOG_INF("TEE Initialized");

    k_msleep(200);

    LOG_INF("Opening Session");

    /* Fill in session_arg */
    memcpy(session_arg.uuid, PUF_TA_UUID, TEE_UUID_LEN);
    memcpy(session_arg.clnt_uuid, CLIENT_UUID, TEE_UUID_LEN);
    session_arg.clnt_login = TEE_IOCTL_LOGIN_USER;
    session_arg.cancel_id  = 0;

    res = tee_open_session(tee_dev, &session_arg, 0, NULL, &session_id);
    if (res != 0 || session_arg.ret != 0) {
        LOG_ERR("tee_open_session() failed: res=%d, TEE_ret=0x%08x", res, session_arg.ret);
        return -1;
    }

    LOG_INF("Session opened: ID = %u", session_id);

    k_msleep(200);

    LOG_INF("Calling PUF_TA_init");

    res = call_puf_ta_init(tee_dev, session_id, init);
    k_msleep(1000); // Give time to process, wait for interrupt
    if (res != 0) {
        LOG_ERR("calling PUF_TA_init failed: res=%d, TEE_ret=0x%08x", res, session_arg.ret);
        return -1;
    }

    LOG_DBG("Copying result of init");
    cpy_last_result(init);

    LOG_INF("Calling PUF_TA_get_commitment");

    res = call_puf_ta_get_commitment(tee_dev, session_id, shm_ptr, commitment);
    k_msleep(1000); // Give time to process, wait for interrupt
    if (res != 0) {
        LOG_ERR("calling PUF_TA_get_commitment failed: res=%d, TEE_ret=0x%08x", res, session_arg.ret);
        return -1;
    }

    LOG_DBG("Copying result of commitment");
    cpy_last_result(init);

    LOG_INF("Calling PUF_TA_get_ZK_proofs");

    res = call_puf_ta_get_zk_proofs(tee_dev, session_id, shm_ptr, proofs);
    k_msleep(1000); // Give time to process, wait for interrupt
    if (res != 0) {
        LOG_ERR("calling PUF_TA_get_ZK_proofs failed: res=%d, TEE_ret=0x%08x", res, session_arg.ret);
        return -1;
    }

    LOG_DBG("Copying result of proofs");
    cpy_last_result(init);

    res = tee_close_session(tee_dev, session_id);
    if (res != 0) {
        LOG_ERR("Failed to close session: %d", res);
    } else {
        LOG_INF("Session closed");
    }

    // End of execution
    return 0;
}
