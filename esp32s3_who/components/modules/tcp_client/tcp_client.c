
#include "tcp_client.h"


#define DEVICE_NAME "THT-Face"
const char *ssid = "Space";
const char *pass = "12345space6789";
const char *TAG_WI_FI = "Wifi Debug";
const char *TAG_FS   = "FS Debug";

#define NOENROL 0
#define ENROLING 1
#define ENROLED 2
#define DUPLICATE 3



uint8_t  CmdEnroll=NOENROL;
char personName[20];
uint32_t personId;



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

void wifi_connection(void) {

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
        ESP_LOGE(TAG_WI_FI, "Failed to get netif handle");
        return;
    }

    const char *hostname;
    esp_err_t err = esp_netif_get_hostname(netif, &hostname);
    if (err == ESP_OK) {
        ESP_LOGI(TAG_WI_FI, "Hostname: %s", hostname);
    } else {
        ESP_LOGE(TAG_WI_FI, "Failed to get hostname: %s", esp_err_to_name(err));
    }
}
// Function to set and print the hostname
void set_and_print_hostname(char *hostName) {

    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif == NULL) {
        ESP_LOGE(TAG_WI_FI, "Failed to get netif handle");
        return;
    }

    // Set the hostname to "THTFace"
    const char *new_hostname = hostName;
    esp_err_t err = esp_netif_set_hostname(netif, new_hostname);
    if (err == ESP_OK) {
        ESP_LOGI(TAG_WI_FI, "Hostname set to: %s", new_hostname);
    } else {
        ESP_LOGE(TAG_WI_FI, "Failed to set hostname: %s", esp_err_to_name(err));
        return;
    }

    // Retrieve and log the hostname to verify
    const char *hostname;
    err = esp_netif_get_hostname(netif, &hostname);
    if (err == ESP_OK) {
        ESP_LOGI(TAG_WI_FI, "Hostname: %s", hostname);
    } else {
        ESP_LOGE(TAG_WI_FI, "Failed to get hostname: %s", esp_err_to_name(err));
    }
}

void socket_task(void *pvParameters) {
    int listen_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        ESP_LOGE(TAGSOCKET, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAGSOCKET, "Socket bind failed: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    if (listen(listen_sock, LISTEN_BACKLOG) < 0) {
        ESP_LOGE(TAGSOCKET, "Socket listen failed: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAGSOCKET, "Socket listening on port %d", PORT);
    int16_t total_received = 0;

    while (1) {
        client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            ESP_LOGE(TAGSOCKET, "Unable to accept connection: errno %d", errno);
            close(listen_sock);
            vTaskDelete(NULL);
            return;
        }

        char rx_buffer[1024];
        while(1){
            int len = recv(client_sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len < 0) {
                ESP_LOGE(TAGSOCKET, "Error receiving data: errno %d", errno);
                break;
            } else if (len == 0) {
                ESP_LOGW(TAGSOCKET, "Connection closed");
                break;
            } else {
                rx_buffer[len] = '\0';
                total_received += len;
                ESP_LOGI(TAGSOCKET, "Received %d bytes: %s", len, rx_buffer);

                // Process received data here
                process_enrollment_command(rx_buffer);
                if(CmdEnroll==ENROLED){
                        // int err = send(client_sock, personId, strlen(ack_message), 0);
                        // if (err < 0) {
                        //     ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
                        // } else {
                        //     ESP_LOGI(TAGSOCKET, "id sent to client\n");
                        // }

                }else if(CmdEnroll==DUPLICATE){

                        // int err = send(client_sock, personId, strlen(ack_message), 0);
                        // if (err < 0) {
                        //     ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
                        // } else {
                        //     ESP_LOGI(TAGSOCKET, "id sent to client\n");
                        // }

                }


                    if (total_received >= ACK_SIZE) {
                        const char *ack_message = "ACK";
                        int err = send(client_sock, ack_message, strlen(ack_message), 0);
                        if (err < 0) {
                            ESP_LOGE(TAGSOCKET, "Error sending ACK: errno %d", errno);
                        } else {
                            ESP_LOGI(TAGSOCKET, "ACK sent to client\n");
                        }
                        total_received = 0; // Reset the counter after sending ACK
                    }

            }
        }
        close(client_sock);
    }
}

void process_enrollment_command(const char* buffer) {

  // Check if the buffer starts with "cmdEnrol" (case-sensitive)
  if (strncmp(buffer, "cmdEnrol", strlen("cmdEnrol")) != 0) {
    // Handle invalid command format
    return;
  }

  // Extract the name (assuming space separates name and ID)
  const char* name_start = buffer + strlen("cmdEnrol") + 1;
  const char* space_pos = strchr(name_start, ' ');
  if (space_pos == NULL) {
    // Handle invalid format (no space)
    return;
  }
  strncpy(personName, name_start, space_pos - name_start);
  personName[space_pos - name_start] = '\0'; // Null terminate the name string

  // Extract the ID (assuming integer after space)
  sscanf(space_pos + 1, "%u", &personId);

  // Process the enrollment data (name and ID)
  // ... (your application logic here)
  // For example, print the information or store it in memory
  printf("Enrolling: Name - %s, ID - %d\n", personName, personId);
  
  CmdEnroll=ENROLING;

}



// void app_main(void) {
//     nvs_flash_init();
//     wifi_connection();
// }
