
#ifdef __cplusplus
extern "C" {
#endif

#include "stomp_client.h"
#include "esp_log.h"


#define STOMP_CONNECT_FRAME     "[\"CONNECT\\naccept-version:1.1\\nheart-beat:10000,10000\\n\\n\\u0000\"]"
#define THT     "wss://grozziieget.zjweiting.com:3091/CloudSocket-Dev/websocket"


static const char *TAG = "stomp_client";

stomp_client_t* stomp_client_init(esp_websocket_client_handle_t ws_client) {
    stomp_client_t *client = malloc(sizeof(stomp_client_t));
    client->ws_client = ws_client;
    // Initialize additional fields as needed
    return client;
}

void stomp_client_connect(stomp_client_t *client) {
    if (client == NULL || client->ws_client == NULL) {
        ESP_LOGE(TAG, "Client or WebSocket client is NULL");
        return;
    }

    // Prepare the STOMP CONNECT frame
    char connect_frame[]= "[\"CONNECT\\naccept-version:1.1\\nheart-beat:10000,10000\\n\\n\\u0000\"]";
    // snprintf(connect_frame, sizeof(connect_frame), STOMP_CONNECT_FRAME, THT);

    ESP_LOGI(TAG, "Sending STOMP CONNECT frame:\n%s", connect_frame);

    // Send the STOMP CONNECT frame over the WebSocket
    if (esp_websocket_client_send_text(client->ws_client, connect_frame, strlen(connect_frame), portMAX_DELAY) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send STOMP CONNECT frame");
        return;
    }

    ESP_LOGI(TAG, "STOMP CONNECT frame sent successfully");
}

void stomp_client_subscribe(stomp_client_t *client, const char *topic, void (*callback)(const char *message)) {
    // Implement subscription logic
}

void stomp_client_send_message(stomp_client_t *client, const char *destination, const char *message) {
    // Implement message sending logic
}

void stomp_client_handle_message(stomp_client_t *client, const char *message) {
    // Handle incoming STOMP messages
    ESP_LOGI(TAG, "Received STOMP message:\n%s", message);

    // Parse and process the STOMP message
    // You can add more code here to handle specific STOMP frames like MESSAGE, CONNECTED, ERROR, etc.
}





#ifdef __cplusplus
}
#endif
