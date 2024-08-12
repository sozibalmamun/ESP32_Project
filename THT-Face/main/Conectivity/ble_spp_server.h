/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BLE_SPP_SERVER_H
#define BLE_SPP_SERVER_H

#include <stdbool.h>
#include "nimble/ble.h"
#include "modlog/modlog.h"



#ifdef __cplusplus
extern "C" {
#endif

/* 16 Bit SPP Service UUID */
#define BLE_SVC_SPP_UUID16                                  0xABF0

/* 16 Bit SPP Service Characteristic UUID */
#define BLE_SVC_SPP_CHR_UUID16                              0xABF1

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;

void startBle(void);

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
int new_gatt_svr_init(void);

/** Misc. */
void print_bytes(const uint8_t *bytes, int len);
void print_addr(const void *addr);

//---------------------------------------------------------------------
int ble_spp_server_gap_event(struct ble_gap_event *event, void *arg);
int gatt_svr_register(void);
void ble_store_config_init(void);


#ifdef __cplusplus
}
#endif

#endif
