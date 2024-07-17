#pragma once

#include <stdio.h>           // For basic printf commands
#include <string.h>          // For handling strings
#include "stdbool.h"
#include <sys/unistd.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"  // For delay, mutexes, semaphores, RTOS operations
#include "esp_system.h"      // For ESP init functions, esp_err_t
#include "esp_wifi.h"        // For esp_wifi_init functions and Wi-Fi operations
#include "esp_log.h"         // For showing logs
#include "esp_event.h"       // For Wi-Fi events
#include "nvs_flash.h"       // Non-volatile storage
#include "lwip/err.h"        // Light weight IP packets error handling
#include "lwip/sys.h"        // System applications for lightweight IP apps
#include "nvs.h"

#include "esp_err.h"
#include "esp_vfs.h"


#include "freertos/task.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"


#define PORT 80
#define LISTEN_BACKLOG 1
#define ACK_SIZE 1024

static const char *TAGSOCKET = "Socket Example";
uint32_t crc_table[256];
uint16_t crc16_table[256];

#ifdef __cplusplus
extern "C"
{
#endif

esp_err_t read_wifi_credentials(char *ssid, size_t ssid_len, char *pass, size_t pass_len) ;
esp_err_t save_wifi_credentials(const char *ssid, const char *pass) ;
void print_hostname();
void set_and_print_hostname();
void wifi_connection(void);

// tcp server
void socket_task(void *pvParameters);

//save wifi info
void save_wifi_info(const char* ssid, const char* pass);
void read_wifi_info(char* ssid, char* pass);


// parsing
void process_command(const char* buffer);
uint32_t crc32(const char *buf, size_t len);
void init_crc32_table();
void init_crc16_table();
uint16_t crc16(const char *buf, size_t len);
uint16_t hex_to_uint16(const char* hex_str);

uint16_t toint2(uint8_t *data_buffer);
void resizeBuffer(void);

#ifdef __cplusplus
}
#endif





