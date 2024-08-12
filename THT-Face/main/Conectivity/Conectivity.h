#ifndef CONECTIVITY_H
#define CONECTIVITY_H


#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_websocket_client.h"


extern uint8_t wifiStatus;
extern esp_websocket_client_handle_t client;


#ifdef __cplusplus
extern "C" {
#endif

void wifi_connection(void);

//save wifi info
// void save_wifi_info(const char* ssid, const char* pass);
// void read_wifi_info(char* ssid, char* pass);
extern uint8_t get_wifi_signal_strength();
// esp_err_t read_wifi_credentials(char *ssid, size_t ssid_len, char *pass, size_t pass_len) ;
// esp_err_t save_wifi_credentials(const char *ssid, const char *pass) ;
// void print_hostname();
// void set_and_print_hostname();
extern uint64_t generate_unique_id(void);
extern void stompAppStart(void);

#ifdef __cplusplus
}
#endif

#endif