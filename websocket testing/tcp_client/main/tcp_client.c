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
#include "esp_spiffs.h"


#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_websocket_client.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#define WEBSOCKET_URI "192.168.1.103:57609"

static const char *TAGSocket = "WEBSOCKET_CLIENT";

static EventGroupHandle_t websocket_event_group;
const int CONNECTED_BIT = BIT0;


#define WIFI_SSID_KEY "wifi_ssid"
#define WIFI_PASS_KEY "wifi_pass"
#define DEVICE_NAME "THT-Face"
const char *ssid = "Space";
const char *pass = "12345space6789";
const char *TAG_WI_FI = "Wifi Debug";
const char *TAG_FS   = "FS Debug";


void save_wifi_info(const char* ssid, const char* pass);
void read_wifi_info(char* ssid, char* pass);
esp_err_t read_wifi_credentials(char *ssid, size_t ssid_len, char *pass, size_t pass_len) ;
esp_err_t save_wifi_credentials(const char *ssid, const char *pass) ;
void print_hostname();
void set_and_print_hostname();

static const char *TAG = "example";

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_id == WIFI_EVENT_STA_START) {
        printf("WIFI CONNECTING....\n");
    } else if (event_id == WIFI_EVENT_STA_CONNECTED) {
        printf("WiFi CONNECTED\n");
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        printf("WiFi lost connection\n");
        esp_wifi_connect();
        printf("Retrying to Connect...\n");
    } else if (event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG_WI_FI, "WiFi got IP: " IPSTR, IP2STR(&event->ip_info.ip));

        uint8_t mac[6];
        esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
        ESP_LOGI(TAG_WI_FI, "MAC address: %02x:%02x:%02x:%02x:%02x:%02x",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        // Read and log the hostname
        print_hostname();
        //set_and_print_hostname("THTFace");

    }
}

void wifi_connection() {

    // Wi-Fi Configuration Phase
    esp_netif_init();
    esp_event_loop_create_default(); // Event loop
    esp_netif_create_default_wifi_sta(); // Wi-Fi station
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = "",
            .password = "",
        }
    };

    strcpy((char *)wifi_configuration.sta.ssid, ssid);
    strcpy((char *)wifi_configuration.sta.password, pass);

    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    
    set_and_print_hostname(DEVICE_NAME);

    // Wi-Fi Start Phase
    esp_wifi_start();
    esp_wifi_set_mode(WIFI_MODE_STA);

    // Wi-Fi Connect Phase
    esp_wifi_connect();
}

void print_hostname() {
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif == NULL) {
        ESP_LOGE(TAG, "Failed to get netif handle");
        return;
    }

    const char *hostname;
    esp_err_t err = esp_netif_get_hostname(netif, &hostname);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Hostname: %s", hostname);
    } else {
        ESP_LOGE(TAG, "Failed to get hostname: %s", esp_err_to_name(err));
    }
}
// Function to set and print the hostname
void set_and_print_hostname(char *hostName) {

    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif == NULL) {
        ESP_LOGE(TAG, "Failed to get netif handle");
        return;
    }

    // Set the hostname to "THTFace"
    const char *new_hostname = hostName;
    esp_err_t err = esp_netif_set_hostname(netif, new_hostname);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Hostname set to: %s", new_hostname);
    } else {
        ESP_LOGE(TAG, "Failed to set hostname: %s", esp_err_to_name(err));
        return;
    }

    // Retrieve and log the hostname to verify
    const char *hostname;
    err = esp_netif_get_hostname(netif, &hostname);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Hostname: %s", hostname);
    } else {
        ESP_LOGE(TAG, "Failed to get hostname: %s", esp_err_to_name(err));
    }
}


void websocket_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAGSocket, "WEBSOCKET_EVENT_CONNECTED");
        xEventGroupSetBits(websocket_event_group, CONNECTED_BIT);
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAGSocket, "WEBSOCKET_EVENT_DISCONNECTED");
        xEventGroupClearBits(websocket_event_group, CONNECTED_BIT);
        // Attempt to reconnect
        while (!esp_websocket_client_is_connected(data->client)) {
            ESP_LOGI(TAGSocket, "Attempting to reconnect...");
            esp_websocket_client_start(data->client);
            vTaskDelay(pdMS_TO_TICKS(10000)); // Wait 10 seconds before retrying
        }
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAGSocket, "WEBSOCKET_EVENT_DATA, Opcode=%d, len=%d", data->op_code, data->data_len);
        if (data->data_len > 0 && data->op_code == 1) {
            ESP_LOGI(TAGSocket, "Received: %.*s", data->data_len, (char *)data->data_ptr);
        }
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAGSocket, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}

void websocket_app_start(void) {
    esp_websocket_client_config_t websocket_cfg = {
        .uri = WEBSOCKET_URI,
        .buffer_size = 8192, // Adjust buffer size as needed
        .task_stack = 8192,  // Adjust stack size as needed
    };

    websocket_event_group = xEventGroupCreate();

    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    esp_websocket_client_start(client);

    // Block for connection
    xEventGroupWaitBits(websocket_event_group, CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    // Example of sending data
    const char *data = "Hello Server";
    esp_websocket_client_send_text(client, data, strlen(data), portMAX_DELAY);

    // For keeping the connection alive, you can implement a keep-alive mechanism
}



void app_main(void) {
   // nvs_flash_init();
    // wifi_connection();
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    wifi_connection();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    websocket_app_start();

}
