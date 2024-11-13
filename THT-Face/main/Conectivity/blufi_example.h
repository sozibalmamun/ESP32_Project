// #pragma once

#ifndef BLUFI_EXAMPLE_H
#define BLUFI_EXAMPLE_H

#include "esp_blufi_api.h"
#include "esp_websocket_client.h"

extern uint8_t sleepEnable;
extern volatile TickType_t sleepTimeOut; 
esp_websocket_client_handle_t client;

#ifdef __cplusplus
extern "C" {
#endif




#define BLUFI_EXAMPLE_TAG "BLUFI_EXAMPLE"
#define BLUFI_INFO(fmt, ...)   ESP_LOGI(BLUFI_EXAMPLE_TAG, fmt, ##__VA_ARGS__)
#define BLUFI_ERROR(fmt, ...)  ESP_LOGE(BLUFI_EXAMPLE_TAG, fmt, ##__VA_ARGS__)

void blufi_dh_negotiate_data_handler(uint8_t *data, int len, uint8_t **output_data, int *output_len, bool *need_free);
int blufi_aes_encrypt(uint8_t iv8, uint8_t *crypt_data, int crypt_len);
int blufi_aes_decrypt(uint8_t iv8, uint8_t *crypt_data, int crypt_len);
uint16_t blufi_crc_checksum(uint8_t iv8, uint8_t *data, int len);

int blufi_security_init(void);
void blufi_security_deinit(void);
int esp_blufi_gap_register_callback(void);
esp_err_t esp_blufi_host_init(void);
esp_err_t esp_blufi_host_and_cb_init(esp_blufi_callbacks_t *callbacks);


void bluFiStart(void);
void deinitBlufi(void);
void blufiAddStart(void);

extern uint8_t wifi_rssi_to_percentage(int32_t rssi);
extern uint64_t generate_unique_id(void);
extern void wssClientInt(void);


#ifdef __cplusplus
}
#endif


#endif