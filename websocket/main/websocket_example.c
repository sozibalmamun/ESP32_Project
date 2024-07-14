/* ESP HTTP Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


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

#define NO_DATA_TIMEOUT_SEC 10

#define THT     "wss://grozziieget.zjweiting.com:3091/CloudSocket-Dev/websocket/"
// #define HOST    "wss://grozziieget.zjweiting.com/"
#define HOST    "grozziieget.zjweiting.com"

#define PORT    3091
#define  PATH  "/CloudSocket-Dev/websocket"

#define STOMP_CONNECT_FRAME     "[\"CONNECT\\naccept-version:1.1\\nheart-beat:10000,10000\\n\\n\\u0000\"]"

static const char *TAG = "example";

esp_websocket_client_handle_t client;

const char echo_org_ssl_ca_cert[]  = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFBzCCA++gAwIBAgIRALIM7VUuMaC/NDp1KHQ76aswDQYJKoZIhvcNAQELBQAw\n" \
"ezELMAkGA1UEBhMCR0IxGzAZBgNVBAgMEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4G\n" \
"A1UEBwwHU2FsZm9yZDEaMBgGA1UECgwRQ29tb2RvIENBIExpbWl0ZWQxITAfBgNV\n" \
"BAMMGEFBQSBDZXJ0aWZpY2F0ZSBTZXJ2aWNlczAeFw0yMjAxMTAwMDAwMDBaFw0y\n" \
"ODEyMzEyMzU5NTlaMFkxCzAJBgNVBAYTAkNOMSUwIwYDVQQKExxUcnVzdEFzaWEg\n" \
"VGVjaG5vbG9naWVzLCBJbmMuMSMwIQYDVQQDExpUcnVzdEFzaWEgUlNBIERWIFRM\n" \
"UyBDQSBHMjCCAaIwDQYJKoZIhvcNAQEBBQADggGPADCCAYoCggGBAKjGDe0GSaBs\n" \
"Yl/VhMaTM6GhfR1TAt4mrhN8zfAMwEfLZth+N2ie5ULbW8YvSGzhqkDhGgSBlafm\n" \
"qq05oeESrIJQyz24j7icGeGyIZ/jIChOOvjt4M8EVi3O0Se7E6RAgVYcX+QWVp5c\n" \
"Sy+l7XrrtL/pDDL9Bngnq/DVfjCzm5ZYUb1PpyvYTP7trsV+yYOCNmmwQvB4yVjf\n" \
"IIpHC1OcsPBntMUGeH1Eja4D+qJYhGOxX9kpa+2wTCW06L8T6OhkpJWYn5JYiht5\n" \
"8exjAR7b8Zi3DeG9oZO5o6Qvhl3f8uGU8lK1j9jCUN/18mI/5vZJ76i+hsgdlfZB\n" \
"Rh5lmAQjD80M9TY+oD4MYUqB5XrigPfFAUwXFGehhlwCVw7y6+5kpbq/NpvM5Ba8\n" \
"SeQYUUuMA8RXpTtGlrrTPqJryfa55hTuX/ThhX4gcCVkbyujo0CYr+Uuc14IOyNY\n" \
"1fD0/qORbllbgV41wiy/2ZUWZQUodqHWkjT1CwIMbQOY5jmrSYGBwwIDAQABo4IB\n" \
"JjCCASIwHwYDVR0jBBgwFoAUoBEKIz6W8Qfs4q8p74Klf9AwpLQwHQYDVR0OBBYE\n" \
"FF86fBEQfgxncWHci6O1AANn9VccMA4GA1UdDwEB/wQEAwIBhjASBgNVHRMBAf8E\n" \
"CDAGAQH/AgEAMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAiBgNVHSAE\n" \
"GzAZMA0GCysGAQQBsjEBAgIxMAgGBmeBDAECATBDBgNVHR8EPDA6MDigNqA0hjJo\n" \
"dHRwOi8vY3JsLmNvbW9kb2NhLmNvbS9BQUFDZXJ0aWZpY2F0ZVNlcnZpY2VzLmNy\n" \
"bDA0BggrBgEFBQcBAQQoMCYwJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmNvbW9k\n" \
"b2NhLmNvbTANBgkqhkiG9w0BAQsFAAOCAQEAHMUom5cxIje2IiFU7mOCsBr2F6CY\n" \
"eU5cyfQ/Aep9kAXYUDuWsaT85721JxeXFYkf4D/cgNd9+hxT8ZeDOJrn+ysqR7NO\n" \
"2K9AdqTdIY2uZPKmvgHOkvH2gQD6jc05eSPOwdY/10IPvmpgUKaGOa/tyygL8Og4\n" \
"3tYyoHipMMnS4OiYKakDJny0XVuchIP7ZMKiP07Q3FIuSS4omzR77kmc75/6Q9dP\n" \
"v4wa90UCOn1j6r7WhMmX3eT3Gsdj3WMe9bYD0AFuqa6MDyjIeXq08mVGraXiw73s\n" \
"Zale8OMckn/BU3O/3aFNLHLfET2H2hT6Wb3nwxjpLIfXmSVcVd8A58XH0g==\n" \
"-----END CERTIFICATE-----\n";


// static TimerHandle_t shutdown_signal_timer;
// static SemaphoreHandle_t shutdown_sema;

// static void shutdown_signaler(TimerHandle_t xTimer)
// {
//     ESP_LOGI(TAG, "No data received for %d seconds, signaling shutdown", NO_DATA_TIMEOUT_SEC);
//     xSemaphoreGive(shutdown_sema);
// }
void stomp_client_connect() {


        // ["CONNECT\naccept-version:1.1\nheart-beat:10000,10000\n\n\u0000"]


    int ret;
    if (0){
        char connect_frame[100] ;//"[\"CONNECT\naccept-version:1.1\nheart-beat:10000,10000\n\n\u0000\"]";
        const char *command = "[\"CONNECT\\n";
        const char *accept_version = "accept-version:1.1\\n";
        const char *heart_beat = "heart-beat:10000,10000\\n\\n";
        char *end_of_frame = "\\u0000\"]";

        // end_of_frame[0] = (char)92;
        snprintf(connect_frame, sizeof(connect_frame), "%s%s%s%s", command, accept_version, heart_beat, end_of_frame);

        ESP_LOGI(TAG, "Sending STOMP CONNECT frame:\n%s", connect_frame);
         ret = esp_websocket_client_send_text(client, connect_frame, sizeof(connect_frame), portMAX_DELAY);


    } else {
    // Send the STOMP CONNECT frame over the WebSocket
        ESP_LOGI(TAG, "Sending STOMP CONNECT frame:\n%s", STOMP_CONNECT_FRAME);
         ret = esp_websocket_client_send_text(client, STOMP_CONNECT_FRAME, sizeof(STOMP_CONNECT_FRAME), portMAX_DELAY);
    }
    if (ret < 0) {
        ESP_LOGE(TAG, "Failed to send STOMP CONNECT frame, error code: %d", ret);
        return;
    }
    ESP_LOGI(TAG, "STOMP CONNECT frame sent successfully");
}

// void stomp_client_handle_message( const char *message) {

//        ESP_LOGI("example", "Received STOMP message:\n%s", message);
//     if (strstr(message, "CONNECTED")) {
//         ESP_LOGI("example", "STOMP CONNECTED");
//         // Subscribe to a topic
//         // stomp_client_subscribe("/topic/cloud", "auto", "subscription-1");
//     } else if (strstr(message, "MESSAGE")) {
//         ESP_LOGI("example", "STOMP MESSAGE");
//         // Handle the received message
//     } else if (strstr(message, "ERROR")) {
//         ESP_LOGE("example", "STOMP ERROR: %s", message);
//     }else if(){


//     }
// }
void stomp_client_subscribe() {
    // Implement subscription logic
    
    char connect_frame[]= "[\"SUBSCRIBE\\nid:sub-0\\ndestination:/topic/cloud\\nack:client\\n\\n\\u0000\"]";
    // snprintf(connect_frame, sizeof(connect_frame), STOMP_CONNECT_FRAME, THT);

    ESP_LOGI(TAG, "Sending STOMP SUBSCRIBE frame:\n%s", connect_frame);

    // Send the STOMP CONNECT frame over the WebSocket
    int ret = esp_websocket_client_send_text(client, connect_frame, strlen(connect_frame), portMAX_DELAY);
    if (ret < 0) {
        ESP_LOGE(TAG, "Failed to send STOMP SUBSCRIBE frame, error code: %d", ret);
        return;
    }
    ESP_LOGI(TAG, "STOMP CONNECT frame sent successfully");
}




static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");

        // stomp_client_connect();     

        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG, "Ping");
        // ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
        if (data->op_code == 0x08 && data->data_len == 2) {
            // ESP_LOGW(TAG, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);
        } else {
            ESP_LOGW(TAG, "Received= %s",(char *)data->data_ptr);
            if((char *)data->data_ptr[0]=='o'){

            stomp_client_connect();
            stomp_client_subscribe();
            

            }     
        }
        // ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);

        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}


const char* stomp_client_socket_url(const char* url) {
    static char socket_url[256];
    snprintf(socket_url, sizeof(socket_url), "%s", url);

    int random1 = esp_random() % 999; // Generates a random number between 0 and 999
    int random2 = esp_random() % 999999; // Generates a random number between 0 and 999999
    snprintf(socket_url + strlen(socket_url), sizeof(socket_url) - strlen(socket_url), "%d/%d/websocket", random1, random2);
    ESP_LOGI(TAG, "Constructed WebSocket URL: %s", socket_url);
    return socket_url;
}

static void websocket_app_start(void)
{
    esp_websocket_client_config_t websocket_cfg = {};

  

    websocket_cfg.uri = stomp_client_socket_url(THT);
    // websocket_cfg.host = HOST;
    // websocket_cfg.port = PORT;
    // websocket_cfg.path = PATH;

    websocket_cfg.use_global_ca_store = true;
    websocket_cfg.skip_cert_common_name_check = true;
    websocket_cfg.disable_auto_reconnect = false;

    ESP_LOGI(TAG, "Initializing global CA store...");
    ESP_ERROR_CHECK(esp_tls_set_global_ca_store((const unsigned char *)echo_org_ssl_ca_cert, sizeof(echo_org_ssl_ca_cert)));


    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    esp_websocket_client_start(client);
    int i = 0;
    while (i < 5) {
        if (esp_websocket_client_is_connected(client)) {




        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    // xSemaphoreTake(shutdown_sema, portMAX_DELAY);
    // esp_websocket_client_close(client, portMAX_DELAY);
    // ESP_LOGI(TAG, "Websocket Stopped");
    // esp_websocket_client_destroy(client);
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("WEBSOCKET_CLIENT", ESP_LOG_DEBUG);
    esp_log_level_set("TRANSPORT_WS", ESP_LOG_DEBUG);
    esp_log_level_set("TRANS_TCP", ESP_LOG_DEBUG);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    websocket_app_start();
}
