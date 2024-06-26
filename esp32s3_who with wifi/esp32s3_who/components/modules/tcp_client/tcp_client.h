#pragma once

#include <stdio.h>           // For basic printf commands
#include <string.h>          // For handling strings
#include "freertos/FreeRTOS.h"  // For delay, mutexes, semaphores, RTOS operations
#include "esp_system.h"      // For ESP init functions, esp_err_t
#include "esp_wifi.h"        // For esp_wifi_init functions and Wi-Fi operations
#include "esp_log.h"         // For showing logs
#include "esp_event.h"       // For Wi-Fi events
#include "nvs_flash.h"       // Non-volatile storage
#include "lwip/err.h"        // Light weight IP packets error handling
#include "lwip/sys.h"        // System applications for lightweight IP apps
#include "nvs.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_vfs.h"

// #define WIFI_SSID_KEY "wifi_ssid"
// #define WIFI_PASS_KEY "wifi_pass"
// #define DEVICE_NAME "THT-Face"
// const char *ssid = "SOZIB";
// const char *pass = "123456789";
// const char *TAG_WI_FI = "Wifi Debug";
// const char *TAG_FS   = "FS Debug";

// static const char *TAG = "example";


#ifdef __cplusplus
extern "C"
{
#endif

esp_err_t read_wifi_credentials(char *ssid, size_t ssid_len, char *pass, size_t pass_len) ;
esp_err_t save_wifi_credentials(const char *ssid, const char *pass) ;
void print_hostname();
void set_and_print_hostname();
void wifi_connection(void);

void save_wifi_info(const char* ssid, const char* pass);
void read_wifi_info(char* ssid, char* pass);

#ifdef __cplusplus
}
#endif





