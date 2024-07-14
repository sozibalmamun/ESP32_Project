
#ifdef __cplusplus
extern "C" {
#endif

#include "stomp_client.h"
#include "esp_log.h"


#define STOMP_CONNECT_FRAME     "[\"CONNECT\\naccept-version:1.1\\nheart-beat:10000,10000\\n\\n\\u0000\"]"

#define THT     "wss://grozziieget.zjweiting.com:3091/CloudSocket-Dev/websocket"


static const char *TAG = "stomp_client";

stomp_client_t* stomp_client_init(esp_websocket_client_handle_t ws_client) {
    ESP_LOGI(TAG, "Initializing STOMP client");

    stomp_client_t *client = malloc(sizeof(stomp_client_t));
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for STOMP client");
        return NULL;
    }
    
    client->ws_client = ws_client;
    return client;
}
 

void stomp_client_connect(stomp_client_t *client) {
    if (client == NULL) {
        ESP_LOGE(TAG, "STOMP client is NULL");
        return;
    }

    if (client->ws_client == NULL) {
        ESP_LOGE(TAG, "WebSocket client is NULL");
        return;
    }

    // Check WebSocket client status
    if (!esp_websocket_client_is_connected(client->ws_client)) {
        ESP_LOGE(TAG, "WebSocket client is not connected");
        return;
    }


        // ["CONNECT\naccept-version:1.1\nheart-beat:10000,10000\n\n\u0000"]

    char connect_frame[100] ;//"[\"CONNECT\naccept-version:1.1\nheart-beat:10000,10000\n\n\u0000\"]";

    const char *command = "[\"CONNECT\n";
    const char *accept_version = "accept-version:1.1\n";
    const char *heart_beat = "heart-beat:10000,10000\n\n";
    char *end_of_frame = "0u0000\"]";
    
    end_of_frame[0] = (char)92;

    snprintf(connect_frame, sizeof(connect_frame), "%s%s%s%s", command, accept_version, heart_beat, end_of_frame);



    ESP_LOGI(TAG, "Sending STOMP CONNECT frame:\n%s", connect_frame);


    // Send the STOMP CONNECT frame over the WebSocket
    int ret = esp_websocket_client_send_text(client->ws_client, STOMP_CONNECT_FRAME, strlen(STOMP_CONNECT_FRAME), portMAX_DELAY);
    if (ret < 0) {
        ESP_LOGE(TAG, "Failed to send STOMP CONNECT frame, error code: %d", ret);
        return;
    }

    ESP_LOGI(TAG, "STOMP CONNECT frame sent successfully");
}

void stomp_client_subscribe(stomp_client_t *client, const char *topic, void (*callback)(const char *message)) {
    // Implement subscription logic
     if (client == NULL) {
        ESP_LOGE(TAG, "STOMP client is NULL");
        return;
    }

    if (client->ws_client == NULL) {
        ESP_LOGE(TAG, "WebSocket client is NULL");
        return;
    }

    // Check WebSocket client status
    if (!esp_websocket_client_is_connected(client->ws_client)) {
        ESP_LOGE(TAG, "WebSocket client is not connected");
        return;
    }

    char connect_frame[]= "[\"SUBSCRIBE\\nid:sub-0\\ndestination:/topic/cloud\\nack:client\\n\\n\\u0000\"]";
    // snprintf(connect_frame, sizeof(connect_frame), STOMP_CONNECT_FRAME, THT);

    ESP_LOGI(TAG, "Sending STOMP SUBSCRIBE frame:\n%s", connect_frame);

    // Send the STOMP CONNECT frame over the WebSocket
    int ret = esp_websocket_client_send_text(client->ws_client, connect_frame, strlen(connect_frame), portMAX_DELAY);
    if (ret < 0) {
        ESP_LOGE(TAG, "Failed to send STOMP SUBSCRIBE frame, error code: %d", ret);
        return;
    }

    ESP_LOGI(TAG, "STOMP CONNECT frame sent successfully");
}

void stomp_client_send_message(stomp_client_t *client, const char *destination, const char *message) {
    // Implement message sending logic
}

void stomp_client_handle_message(stomp_client_t *client, const char *message) {

       ESP_LOGI("example", "Received STOMP message:\n%s", message);
    if (strstr(message, "CONNECTED")) {
        ESP_LOGI("example", "STOMP CONNECTED");
        // Subscribe to a topic
        stomp_client_subscribe("/topic/cloud", "auto", "subscription-1");
    } else if (strstr(message, "MESSAGE")) {
        ESP_LOGI("example", "STOMP MESSAGE");
        // Handle the received message
    } else if (strstr(message, "ERROR")) {
        ESP_LOGE("example", "STOMP ERROR: %s", message);
    }
}





#ifdef __cplusplus
}
#endif
