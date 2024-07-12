#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_event.h"
#include "esp_tls.h"


#define WEBSOCKET_URI "wss://grozziieget.zjweiting.com:3091/CloudSocket-Dev/websocket/"

static const char *TAG = "example";

void websocket_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data)
{
    // Handle WebSocket events here
    // You can adapt this function to integrate with your StompClient functions
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize TCP/IP and WiFi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect()); // This connects to WiFi using configuration in sdkconfig

    // Configure the WebSocket client
    esp_websocket_client_config_t websocket_cfg = {
        .uri = WEBSOCKET_URI,
    };

    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);

    // Initialize StompClient
    stomp_client_t *stomper = stomp_client_init(client);
    stomp_client_connect(stomper);

    // Implement message handling and subscriptions using the stomp_client functions
}
