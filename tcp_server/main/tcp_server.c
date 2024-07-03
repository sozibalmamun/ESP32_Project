#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"




#define SERVER_IP "192.168.1.103"  // Replace with your server's IP address
#define SERVER_PORT 57609
#define TIMER_PERIOD pdMS_TO_TICKS(60000)  // 1 minute

#define WIFI_SSID "Space"
#define WIFI_PASS "12345space6789"
#define PORT 80
#define LISTEN_BACKLOG 1
#define ACK_SIZE 1024

static const char *TAG = "Socket Example";

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Connecting to Wi-Fi...");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi disconnected, reconnecting...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void wifi_init_sta() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void socket_task(void *pvParameters) {
    int listen_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Socket bind failed: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    if (listen(listen_sock, LISTEN_BACKLOG) < 0) {
        ESP_LOGE(TAG, "Socket listen failed: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Socket listening on port %d", PORT);
    int16_t total_received = 0;

    while (1) {
        client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            close(listen_sock);
            vTaskDelete(NULL);
            return;
        }

        char rx_buffer[1024];
        while(1){
            int len = recv(client_sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len < 0) {
                ESP_LOGE(TAG, "Error receiving data: errno %d", errno);
                break;
            } else if (len == 0) {
                ESP_LOGW(TAG, "Connection closed");
                break;
            } else {
                rx_buffer[len] = '\0';
                total_received += len;
                ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
                // Process received data here
                    if (total_received >= ACK_SIZE) {
                        const char *ack_message = "ACK";
                        int err = send(client_sock, ack_message, strlen(ack_message), 0);
                        if (err < 0) {
                            ESP_LOGE(TAG, "Error sending ACK: errno %d", errno);
                        } else {
                            ESP_LOGI(TAG, "ACK sent to client\n");
                        }
                        total_received = 0; // Reset the counter after sending ACK
                    }

            }
        }
        close(client_sock);
    }
}

void send_data_task(void *pvParameters) {
    int sock;
    struct sockaddr_in dest_addr;
    fd_set readfds;
    struct timeval timeout;
    char rx_buffer[1024];

    while (1) {
        // Create socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            vTaskDelay(pdMS_TO_TICKS(1000));  // Retry every 1 second if failed to create socket
            continue;
        }

        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP, &dest_addr.sin_addr);

        // Connect to server
        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(1000));  // Retry every 1 second if failed to connect
            continue;
        }

        // Keep the connection alive and send data every minute
        while (1) {
            const char *payload = "Periodic data payload";
            err = send(sock, payload, strlen(payload), 0);
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;  // Break out of the loop if sending failed
            } else {
                ESP_LOGI(TAG, "Message sent: %s", payload);
            }

            // Set up the select call for receiving data
            FD_ZERO(&readfds);
            FD_SET(sock, &readfds);

            timeout.tv_sec = 60;  // 1 minute timeout
            timeout.tv_usec = 0;

            int activity = select(sock + 1, &readfds, NULL, NULL, &timeout);

            if (activity < 0) {
                ESP_LOGE(TAG, "Select error: errno %d", errno);
                break;
            }

            if (FD_ISSET(sock, &readfds)) {
                int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
                if (len < 0) {
                    ESP_LOGE(TAG, "Error receiving data: errno %d", errno);
                    break;
                } else if (len == 0) {
                    ESP_LOGW(TAG, "Connection closed");
                    break;
                } else {
                    rx_buffer[len] = '\0';
                    ESP_LOGI(TAG, "Received from server: %s", rx_buffer);
                }
            } else {
                ESP_LOGI(TAG, "Timeout occurred, no data received");
            }
        }

        close(sock);  // Close the socket if sending failed or connection closed
    }
}

void app_main() {
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();
    xTaskCreate(&socket_task, "socket_task", 4096, NULL, 5, NULL);
    xTaskCreate(&send_data_task, "send_data_task", 4096, NULL, 5, NULL);

}
