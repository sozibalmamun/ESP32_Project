
#include "tcp_client.h"
#include "string.h"


#define DEVICE_NAME "THT-Face"
const char *ssid = "Space";
const char *pass = "12345space6789";
const char *TAG_WI_FI = "Wifi Debug";
const char *TAG_FS   = "FS Debug";

#define IDLEENROL 0
#define ENROLING 0x01
#define ENROLED 0x02
#define DUPLICATE 0x03

#define TIMEOUT_50_MS         5
#define TIMEOUT_100_MS        10
#define TIMEOUT_120_MS        12
#define TIMEOUT_150_MS        15
#define TIMEOUT_200_MS        20
#define TIMEOUT_300_MS        30
#define TIMEOUT_500_MS        50
#define TIMEOUT_1000_MS       100
#define TIMEOUT_2000_MS       200
#define TIMEOUT_3000_MS       300
#define TIMEOUT_4000_MS       400
#define TIMEOUT_5000_MS       500
#define TIMEOUT_6000_MS       600
#define TIMEOUT_7000_MS       700
#define TIMEOUT_9000_MS       900
#define TIMEOUT_10000_MS      1000
#define TIMEOUT_12000_MS      1200
#define TIMEOUT_20000_MS      2000
#define TIMEOUT_15_S          1500
#define TIMEOUT_30_S          3000
#define TIMEOUT_45_S          4500
#define TIMEOUT_1_MIN         6000
#define TIMEOUT_2_MIN         12000
#define TIMEOUT_5_MIN         30000


volatile uint8_t  CmdEnroll=IDLEENROL;
char personName[20];
uint16_t personId;
char tcpBuffer[2024]; // Adjust MAX_TRANSACTION_SIZE as needed
TickType_t erolTimeOut;


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
       // ESP_LOGE(TAGSOCKET, "Socket listen failed: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

   // ESP_LOGI(TAGSOCKET, "Socket listening on port %d", PORT);
    int16_t total_received = 0;

    while (1) {

        client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            ESP_LOGE(TAGSOCKET, "Unable to accept connection: errno %d", errno);
            close(listen_sock);
            vTaskDelete(NULL);
            return;
        }
        // Send success message to the client
        send(client_sock, "200!\n\r", 7, 0);


        char rx_buffer[500];
        while(1){

            uint16_t len = recv(client_sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len < 0) {
               // ESP_LOGE(TAGSOCKET, "Error receiving data: errno %d", errno);
                break;
            } else if (len == 0) {
                ESP_LOGW(TAGSOCKET, "Connection closed");
                break;
            } else {
                rx_buffer[len] = '\0';
                total_received += len;
                // ESP_LOGI(TAGSOCKET, "Received %d bytes: %s", len, rx_buffer);
                uint16_t tcpLen = strlen(tcpBuffer);

                memcpy(&tcpBuffer[tcpLen],&rx_buffer ,len);

                // printf("\ntcp len %d  buff %s",tcpLen,tcpBuffer);
                // Process received data here
                if(strlen(tcpBuffer)>5)resizeBuffer();

                if (strstr(tcpBuffer, "cmd") && strstr(tcpBuffer, "End") && strstr(tcpBuffer, "End") > strstr(tcpBuffer, "cmd")) {// varifi the cmd pattern

                // if (strstr(tcpBuffer, "cmd") != NULL) {

                    process_command(tcpBuffer);
                    bool cmd=true;
                   while(cmd){

                        if(CmdEnroll==ENROLED){

                            char personIdStr[12]; // assuming 32-bit uint can be represented in 11 chars + null terminator
                            snprintf(personIdStr, sizeof(personIdStr), "%u", personId);
                            int err = send(client_sock, personIdStr, strlen(personIdStr), 0);
                            if (err < 0) {
                              //  ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
                            } else {
                                ESP_LOGI(TAGSOCKET, "id sent to client\n");
                                CmdEnroll = IDLEENROL;
                                cmd=false;
                            }
                        }else if(CmdEnroll==DUPLICATE){

                            ESP_LOGI(TAGSOCKET, "duplicate ack\n");

                            const char *ack_message = "NDP";// nack for duplicate person
                            int err = send(client_sock, ack_message, strlen(ack_message), 0);
                            if (err < 0) {
                                //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
                            } else {
                                ESP_LOGI(TAGSOCKET, "back to idle mode\n");
                                CmdEnroll = IDLEENROL;
                                cmd=false;
                            }
                        }else {

                            TickType_t TimeOut = xTaskGetTickCount();
                    
                            if (TimeOut-erolTimeOut> TIMEOUT_15_S ){
                           // ESP_LOGI(TAGSOCKET, "not acking\n");
                           // send(client_sock, "\nwait for..", 8, 0);
                            CmdEnroll = IDLEENROL;

                            const char *ack_message = "NETO";// nack for time out
                            int err = send(client_sock, ack_message, strlen(ack_message), 0);
                            if (err < 0) {
                                //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
                            } else {
                                ESP_LOGI(TAGSOCKET, "back to idle mode\n");
                                cmd=false;
                            }
                            printf("\ncmd enroll flag status %d",CmdEnroll);
                            vTaskDelay(10);

                            }
                        }

                    }
                }else{ 

                    ESP_LOGE(TAGSOCKET, "invalid %d", errno);

                    }
                    // if (total_received >= ACK_SIZE) {
                    //     const char *ack_message = "ACK";
                    //     int err = send(client_sock, ack_message, strlen(ack_message), 0);
                    //     if (err < 0) {
                    //         ESP_LOGE(TAGSOCKET, "Error sending ACK: errno %d", errno);
                    //     } else {
                    //         ESP_LOGI(TAGSOCKET, "ACK sent to client\n");
                    //     }
                    //     total_received = 0; // Reset the counter after sending ACK
                    // }

            }
        }
        close(client_sock);
    }
}

void process_command(const char* buffer) {
    
    
    // if(strlen(buffer)>10)resizeBuffer();
  // Check if the buffer starts with "cmdEnrol" (case-sensitive)
    if (strncmp(buffer, "cmdEnrol", strlen("cmdEnrol")) == 0) {
       
        init_crc16_table();
        // Extract the name (assuming space separates name and ID)
        const char* name_start = buffer + strlen("cmdEnrol") + 1;
        const char* space_pos = strchr(name_start, ' ');
        if (space_pos == NULL) {
            // Handle invalid format (no space)
            return;
        }
        strncpy(personName, name_start, space_pos - name_start);
        personName[space_pos - name_start] = '\0'; // Null terminate the name string

        // Extract the 4-character hex CRC
        char crc_str[5];
        strncpy(crc_str, space_pos + 1, 4);
        crc_str[4] = '\0'; // Null terminate the CRC string
        // Convert the hex string to a 16-bit integer
        uint16_t rxCrc = hex_to_uint16(crc_str);

        // Check for end command string (case-sensitive)
        const char* end_cmd_pos = strstr(buffer, "cmdEnd");
        if (end_cmd_pos != NULL) {
            // Data reception complete, print information
            printf("Enrollment data received:\n");
            printf("  - CRC RCV: %x\n",rxCrc);

            uint16_t calculated_crc = crc16(personName, strlen(personName));
            printf("  - CRC16 CALCULATED: %x\n", calculated_crc);

            if (calculated_crc == rxCrc) {
                printf("\ncmd enroll flag status %d",CmdEnroll);

                CmdEnroll = ENROLING;
                erolTimeOut = xTaskGetTickCount();
                printf("CRC check passed.\n");
                printf("  - Name: %s\n", personName);
                memset(tcpBuffer, 0, strlen(tcpBuffer));
                return;
            } else {
                printf("CRC check failed.\n");
                memset(tcpBuffer, 0, strlen(tcpBuffer));
                CmdEnroll = IDLEENROL;
                return;
            }
        }else{

        }

    }else{
    }
}
void resizeBuffer() {
    char startMarker[] = "cmdEnrol";
    char endMarker[] = "cmdEnd";
    
    char *start = strstr(tcpBuffer, startMarker);
    char *end = strstr(tcpBuffer, endMarker);
    
    if (start && end && end > start) {
        end += strlen(endMarker); // Move pointer to end of endMarker
        
        size_t newSize = end - start;
        char newBuffer[newSize + 1];
        strncpy(newBuffer, start, newSize);
        newBuffer[newSize] = '\0'; // Null terminate the new buffer
        
        printf("New Buffer: %s\n", newBuffer);
        
        // Optionally, if you want to update the global tcpBuffer with resized data
        memset(tcpBuffer, 0, sizeof(tcpBuffer));
        strncpy(tcpBuffer, newBuffer, newSize);
    } else {
        printf("Markers not found or in wrong order\n");

    }
}



// CRC-32 table for faster computation
void init_crc32_table() {
    uint32_t polynomial = 0xEDB88320;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            if (c & 1) {
                c = polynomial ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        crc_table[i] = c;
    }
}

uint32_t crc32(const char *buf, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        uint8_t byte = buf[i];
        crc = crc_table[(crc ^ byte) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}


// CRC-16-CCITT table for faster computation

void init_crc16_table() {
    uint16_t polynomial = 0x1021;
    for (uint16_t i = 0; i < 256; i++) {
        uint16_t c = i << 8;
        for (int j = 0; j < 8; j++) {
            if (c & 0x8000) {
                c = (c << 1) ^ polynomial;
            } else {
                c = c << 1;
            }
        }
        crc16_table[i] = c;
    }
}

uint16_t crc16(const char *buf, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        uint8_t byte = buf[i];
        crc = crc16_table[((crc >> 8) ^ byte) & 0xFF] ^ (crc << 8);
    }
    return crc;
}

uint16_t hex_to_uint16(const char* hex_str) {
    uint16_t result=0x0000;
    sscanf(hex_str, "%4hx", &result);
    return result;
}


uint32_t toint4(uint8_t *data_buffer) {
  uint32_t slotL = data_buffer[0];
  slotL = (slotL << 8) + data_buffer[1];
  slotL = (slotL << 8) + data_buffer[2];
  slotL = (slotL << 8) + data_buffer[3];

  return slotL;
}