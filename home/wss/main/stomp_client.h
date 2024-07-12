#ifndef STOMP_CLIENT_H
#define STOMP_CLIENT_H

#include "esp_websocket_client.h"

typedef struct {
    esp_websocket_client_handle_t ws_client;
    // Add additional fields as needed
} stomp_client_t;

stomp_client_t* stomp_client_init(esp_websocket_client_handle_t ws_client);
void stomp_client_connect(stomp_client_t *client);
void stomp_client_subscribe(stomp_client_t *client, const char *topic, void (*callback)(const char *message));
void stomp_client_send_message(stomp_client_t *client, const char *destination, const char *message);

#endif // STOMP_CLIENT_H
