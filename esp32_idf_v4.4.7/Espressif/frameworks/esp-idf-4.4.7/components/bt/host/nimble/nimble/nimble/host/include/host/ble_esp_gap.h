/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#ifndef H_BLE_ESP_GAP_
#define H_BLE_ESP_GAP_

#ifdef __cplusplus
extern "C" {
#endif

enum gap_status {
    BLE_GAP_STATUS_ADV = 0,
    BLE_GAP_STATUS_EXT_ADV,
    BLE_GAP_STATUS_SCAN,
    BLE_GAP_STATUS_CONN,
    BLE_GAP_STATUS_PAIRED,
    BLE_GAP_STATUS_GATTS,
    BLE_GAP_STATUS_HOST_PRIVACY,
    BLE_GAP_STATUS_PERIODIC,
};

typedef enum gap_status gap_status_t;

/**
 * This API gives the current status of various stack operations
 *
 * @return                      0 on success; nonzero bits indicating different
 *                              operations as per enum gap_status.
 */
int ble_gap_host_check_status(void);

#ifdef __cplusplus
}
#endif

#endif
