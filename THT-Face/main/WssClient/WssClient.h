#ifndef STOMECLIENT_H
#define STOMECLIENT_H

#include <stdio.h>
#include <string.h>          // For handling strings
#include "stdbool.h"
#include <sys/unistd.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_websocket_client.h"


#include "lwip/err.h"        // Light weight IP packets error handling
#include "lwip/sys.h"        // System applications for lightweight IP apps
#include "nvs.h"
#include "esp_err.h"
#include "esp_vfs.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "globalScope.h"



#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "esp_crt_bundle.h"
#include "esp_websocket_client.h"
#include "esp_log.h"
#include "esp_heap_caps.h" 

#include "mbedtls/base64.h"  

#define TAG "WSS_CLIENT"

// WebSocket client context
typedef struct {
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;
} wss_client_t;

uint8_t *ssl_cert_pem = NULL;  // Dynamic allocation for SSL certificate


#ifdef __cplusplus
extern "C" {
#endif


esp_websocket_client_handle_t client;


const DATA_FLASH char echo_org_ssl_ca_cert[] =\

"-----BEGIN CERTIFICATE-----\n" \
"MIIErjCCA5agAwIBAgIQBYAmfwbylVM0jhwYWl7uLjANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0xNzEyMDgxMjI4MjZaFw0yNzEyMDgxMjI4MjZaMHIxCzAJBgNVBAYTAkNO\n" \
"MSUwIwYDVQQKExxUcnVzdEFzaWEgVGVjaG5vbG9naWVzLCBJbmMuMR0wGwYDVQQL\n" \
"ExREb21haW4gVmFsaWRhdGVkIFNTTDEdMBsGA1UEAxMUVHJ1c3RBc2lhIFRMUyBS\n" \
"U0EgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCgWa9X+ph+wAm8\n" \
"Yh1Fk1MjKbQ5QwBOOKVaZR/OfCh+F6f93u7vZHGcUU/lvVGgUQnbzJhR1UV2epJa\n" \
"e+m7cxnXIKdD0/VS9btAgwJszGFvwoqXeaCqFoP71wPmXjjUwLT70+qvX4hdyYfO\n" \
"JcjeTz5QKtg8zQwxaK9x4JT9CoOmoVdVhEBAiD3DwR5fFgOHDwwGxdJWVBvktnoA\n" \
"zjdTLXDdbSVC5jZ0u8oq9BiTDv7jAlsB5F8aZgvSZDOQeFrwaOTbKWSEInEhnchK\n" \
"ZTD1dz6aBlk1xGEI5PZWAnVAba/ofH33ktymaTDsE6xRDnW97pDkimCRak6CEbfe\n" \
"3dXw6OV5AgMBAAGjggFPMIIBSzAdBgNVHQ4EFgQUf9OZ86BHDjEAVlYijrfMnt3K\n" \
"AYowHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUwDgYDVR0PAQH/BAQD\n" \
"AgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjASBgNVHRMBAf8ECDAG\n" \
"AQH/AgEAMDQGCCsGAQUFBwEBBCgwJjAkBggrBgEFBQcwAYYYaHR0cDovL29jc3Au\n" \
"ZGlnaWNlcnQuY29tMEIGA1UdHwQ7MDkwN6A1oDOGMWh0dHA6Ly9jcmwzLmRpZ2lj\n" \
"ZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RDQS5jcmwwTAYDVR0gBEUwQzA3Bglg\n" \
"hkgBhv1sAQIwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29t\n" \
"L0NQUzAIBgZngQwBAgEwDQYJKoZIhvcNAQELBQADggEBAK3dVOj5dlv4MzK2i233\n" \
"lDYvyJ3slFY2X2HKTYGte8nbK6i5/fsDImMYihAkp6VaNY/en8WZ5qcrQPVLuJrJ\n" \
"DSXT04NnMeZOQDUoj/NHAmdfCBB/h1bZ5OGK6Sf1h5Yx/5wR4f3TUoPgGlnU7EuP\n" \
"ISLNdMRiDrXntcImDAiRvkh5GJuH4YCVE6XEntqaNIgGkRwxKSgnU3Id3iuFbW9F\n" \
"UQ9Qqtb1GX91AJ7i4153TikGgYCdwYkBURD8gSVe8OAco6IfZOYt/TEwii1Ivi1C\n" \
"qnuUlWpsF1LdQNIdfbW3TSe0BhQa7ifbVIfvPWHYOu3rkg1ZeMo6XRU9B4n5VyJY\n" \
"RmE=\n" \
"-----END CERTIFICATE-----\n" \
"---\n";








#define MIN(a, b) (((a) < (b)) ? (a) : (b))

extern uint8_t networkStatus;
extern const char* day_names[];
extern bool dspTimeFormet;
extern uint16_t batVoltage;
extern uint8_t config;

//ble
extern bool ble_is_connected;

extern void blufi_security_deinit(void);
extern void blufiAddStart(void);
extern esp_err_t blufi_security_init(void);
extern void send_custom_data_to_app(const char *data);
//-----------------------------------------
extern uint64_t generate_unique_id(void);
extern void process_command(const char* buffer);
extern void dataHandele(const char *rx_buffer);
extern bool imagesent(uint8_t *buff, uint16_t buffLen, uint8_t h, uint8_t w ,char* name,uint16_t id);
extern uint16_t crc16(const char *buf, size_t len);
uint8_t wifi_rssi_to_percentage(int32_t rssi);





// int perform_mbedtls_handshake();
void wssClientInt(void);
void wssReset(void);
bool sendToWss(uint8_t *buff, size_t buffLen);
void process_data(char* data, uint32_t len) ;

#ifdef __cplusplus
}
#endif



#endif
