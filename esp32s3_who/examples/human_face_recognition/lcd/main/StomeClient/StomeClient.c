#include "StomeClient.h"

const char *TAG = "WEBSOCKET";
const char *TAGSTOMP = "STOMP_CLIENT";
const char *TAG_WI_FI = "Wifi Debug";
const char *TAG_FS   = "FS Debug";

const char *ssid = "Space";
const char *pass = "12345space6789";




extern volatile uint8_t CmdEnroll;
extern char personName[20];
extern uint16_t personId;

// volatile uint8_t  CmdEnroll=IDLEENROL;
// char personName[20];
// uint16_t personId;


char tcpBuffer[2024]; // Adjust MAX_TRANSACTION_SIZE as needed
TickType_t erolTimeOut;


static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_id == WIFI_EVENT_STA_START) {
        printf("WIFI CONNECTING....\n");
    } else if (event_id == WIFI_EVENT_STA_CONNECTED) {

        printf("WiFi CONNECTED\n");

        stompAppStart();

    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        // printf("WiFi lost connection\n");
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
        // print_hostname();
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






static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");

        break;
    case WEBSOCKET_EVENT_DATA:
        if (data->op_code == 0x08 && data->data_len == 2) {
            // ESP_LOGW(TAG, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);
        } else {

            ESP_LOGW(TAG, "Received= %s",(char *)data->data_ptr);

            if(data->data_ptr[0]=='o'){
            stomp_client_connect();            
            }  
            else if(data->data_ptr[0]=='a'){
                stomp_client_handle_message(&data->data_ptr[3]);
            }else if(data->data_ptr[0]=='h'){
                ESP_LOGI(TAG, "Ping");
                if(!stompSend((char*)testdata,"/app/cloud")){
                ESP_LOGI(TAGSTOMP, "Data sending error");

                }
            }else if(data->data_ptr[0]=='c'){

                ESP_LOGW(TAG, "Received= %s",(char *)data->data_ptr);
                stompAppStart();
            }
        }
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}



// void app_main(void)
// {
//     ESP_ERROR_CHECK(nvs_flash_init());
//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());
//     ESP_ERROR_CHECK(example_connect());

//     websocket_app_start();
// }


void stomp_client_int( stompInfo_cfg_t stompSetup ) {
   
    esp_websocket_client_config_t websocket_cfg = {};

    char socket[100];
    snprintf(socket, sizeof(socket), "%s", stompSetup.uri);

    int random1 = esp_random() % 999; // Generates a random number between 0 and 999
    int random2 = esp_random() % 999999; // Generates a random number between 0 and 999999
    snprintf(socket + strlen(socket), sizeof(socket) - strlen(socket), "%d/%d/websocket", random1, random2);

    websocket_cfg.uri = (const char*)socket;
    // memset(socket,0,strlen(socket));

    // snprintf(socket, sizeof(socket), "%s", stompSetup.path);
    // snprintf(socket + strlen(socket), sizeof(socket) - strlen(socket), "%d/%d/websocket", random1, random2);
    
    // websocket_cfg.path = (const char*)socket;// path
    // websocket_cfg.port = stompSetup.port;
    // websocket_cfg.host = (const char*)stompSetup.host;

    // websocket_cfg.use_global_ca_store = false;// try 
    // websocket_cfg.skip_cert_common_name_check = false;
    // websocket_cfg.disable_auto_reconnect = false;

    websocket_cfg.use_global_ca_store = true;// ok 
    websocket_cfg.skip_cert_common_name_check = true;
    websocket_cfg.disable_auto_reconnect = false;


    ESP_LOGI(TAG, "Constructed WebSocket URL: %s", websocket_cfg.uri);
    // ESP_LOGI(TAG, "Constructed WebSocket PATH: %s", websocket_cfg.path);


    ESP_LOGI(TAG, "Initializing global CA store...");
    ESP_ERROR_CHECK(esp_tls_set_global_ca_store((const unsigned char *)echo_org_ssl_ca_cert, sizeof(echo_org_ssl_ca_cert)));


    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);
}

void stompAppStart(void)
{

    stompInfo_cfg_t stompInfo ={
        .uri = THT,
        .host = HOST,
        .port = PORT,
        .path = PATH
    };
    stomp_client_int(stompInfo);
}

void stomp_client_connect() {

    char connect_frame[100] ;//"[\"CONNECT\naccept-version:1.1\nheart-beat:10000,10000\n\n\u0000\"]";
    snprintf(connect_frame, sizeof(connect_frame), "%s%s%s%s", "[\"CONNECT\\n", "accept-version:1.1\\n", "heart-beat:10000,10000\\n\\n", "\\u0000\"]");
    // ESP_LOGI(TAGSTOMP, "Sending STOMP CONNECT frame:\n%s", connect_frame);
    esp_websocket_client_send_text(client, connect_frame, strlen(connect_frame), portMAX_DELAY);
}
void stomp_client_subscribe(char* topic) {
    
    //example pack "[\"SUBSCRIBE\\nid:sub-0\\ndestination:/topic/cloud\\nack:client\\n\\n\\u0000\"]";

    char connect_frame[100] ;

    // snprintf(connect_frame, sizeof(connect_frame), "%s%s%s%s%s%s%s", "[\"SUBSCRIBE\\n","id:sub-0\\n", "destination:",topic,"\\n", "ack:client\\n\\n", "\\u0000\"]");
    
    snprintf(connect_frame, sizeof(connect_frame), "[\"SUBSCRIBE\\nid:sub-0\\ndestination:%s\\nack:client\\n\\n\\u0000\"]", topic);

    // ESP_LOGI(TAGSTOMP, "Sending STOMP Subscribe frame :\n%s", connect_frame);
    esp_websocket_client_send_text(client, connect_frame, strlen(connect_frame), portMAX_DELAY);
}
bool stompSend(char * buff, char* topic){


    char tempFrame[CHANK_SIZE+1]; 
    memset(tempFrame,0,sizeof(tempFrame));

    uint16_t currentIndex=0;
    uint16_t buffLen =strlen(buff);
    // ESP_LOGI(TAGSTOMP, "Sending  total len :%d\n", buffLen);

    do{
        memset(tempFrame,0,sizeof(tempFrame));
        if(buffLen<=CHANK_SIZE){
            currentIndex ? memcpy(&tempFrame,&buff[currentIndex-1],buffLen) : memcpy(&tempFrame,&buff[currentIndex],buffLen);
            // memcpy(&tempFrame,&buff[currentIndex-1],buffLen);
            buffLen= buffLen - buffLen;
        }else{

            currentIndex ? memcpy(&tempFrame,&buff[currentIndex-1],sizeof(tempFrame)-1) : memcpy(&tempFrame,&buff[currentIndex],sizeof(tempFrame)-1);
            // memcpy(&tempFrame,&buff[currentIndex-1],sizeof(tempFrame));
            currentIndex+= CHANK_SIZE;
            buffLen= buffLen - CHANK_SIZE;
        }
        tempFrame[strlen(tempFrame)] = '\0';  // Null-terminate the chunk


        char connect_frame[strlen(tempFrame)+37+strlen(topic)];memset(connect_frame,0,sizeof(connect_frame));

        // ESP_LOGI(TAGSTOMP, "Sending  tempFrame len :%d dynamic pac len %d\n", strlen(tempFrame) ,sizeof(connect_frame));

        snprintf(connect_frame, sizeof(connect_frame), "[\"SEND\\ndestination:%s\\n\\n%s\\n\\n\\u0000\"]", topic, tempFrame);
        // ESP_LOGI(TAGSTOMP, "Sending  connect_frame len :%d\n", strlen(connect_frame));

        ESP_LOGI(TAGSTOMP, "Sending STOMP MSG :\n%s", connect_frame);

        if(!esp_websocket_client_is_connected(client))return false;

        if(esp_websocket_client_send_text(client, connect_frame, strlen(connect_frame), portMAX_DELAY)!=ESP_OK){
            // ESP_LOGI(TAGSTOMP, "Sending STOMP   sent len :%d  remain   %d\n", currentIndex,buffLen);
        }else return false;

    }while(buffLen!=0);


return true;

}


void stomp_client_handle_message( const char *message) {

    // ESP_LOGI(TAGSTOMP, "Received STOMP message:\n%s", message);
    if (strstr(message, "CONNECTED")) {
        ESP_LOGI(TAGSTOMP, "STOMP CONNECTED");
        // Subscribe to a topic
        stomp_client_subscribe("/topic/cloud");
    } else if (strstr(message, "MESSAGE")) {
        ESP_LOGI(TAGSTOMP, "STOMP MESSAGE");
        
        // if(!stompSend(testdata,"/app/cloud"))ESP_LOGI(TAGSTOMP, "Data sending error");
        // socket_task(message);

        // Handle the received message
    } else if (strstr(message, "ERROR")) {

        ESP_LOGE(TAGSTOMP, "STOMP ERROR: %s", message);

    }
}


// void socket_task(const char *rx_buffer) {


//             uint16_t len = recv(client_sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
//             if (len < 0) {
//                // ESP_LOGE(TAGSOCKET, "Error receiving data: errno %d", errno);
//                 break;
//             } else if (len == 0) {
//                 ESP_LOGW(TAGSOCKET, "Connection closed");
//                 break;
//             } else {
//                 rx_buffer[len] = '\0';
//                 total_received += len;
//                 // ESP_LOGI(TAGSOCKET, "Received %d bytes: %s", len, rx_buffer);
//                 uint16_t tcpLen = strlen(tcpBuffer);

//                 memcpy(&tcpBuffer[tcpLen],&rx_buffer ,len);

//                 // printf("\ntcp len %d  buff %s",tcpLen,tcpBuffer);
//                 // Process received data here
//                 if(strlen(tcpBuffer)>5)resizeBuffer();

//                 if (strstr(tcpBuffer, "cmd") && strstr(tcpBuffer, "End") && strstr(tcpBuffer, "End") > strstr(tcpBuffer, "cmd")) {// varifi the cmd pattern

//                 // if (strstr(tcpBuffer, "cmd") != NULL) {

//                     process_command(tcpBuffer);
//                     bool cmd=true;
//                    while(cmd){

//                         if(CmdEnroll==ENROLED){

//                             char personIdStr[12]; // assuming 32-bit uint can be represented in 11 chars + null terminator
//                             snprintf(personIdStr, sizeof(personIdStr), "%u", personId);
//                             int err = send(client_sock, personIdStr, strlen(personIdStr), 0);
//                             if (err < 0) {
//                               //  ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
//                             } else {
//                                 ESP_LOGI(TAGSOCKET, "id sent to client\n");
//                                 CmdEnroll = IDLEENROL;
//                                 cmd=false;
//                             }
//                         }else if(CmdEnroll==DUPLICATE){

//                             ESP_LOGI(TAGSOCKET, "duplicate ack\n");

//                             const char *ack_message = "NDP";// nack for duplicate person
//                             int err = send(client_sock, ack_message, strlen(ack_message), 0);
//                             if (err < 0) {
//                                 //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
//                             } else {
//                                 ESP_LOGI(TAGSOCKET, "back to idle mode\n");
//                                 CmdEnroll = IDLEENROL;
//                                 cmd=false;
//                             }
//                         }else {

//                             TickType_t TimeOut = xTaskGetTickCount();
                    
//                             if (TimeOut-erolTimeOut> TIMEOUT_15_S ){
//                            // ESP_LOGI(TAGSOCKET, "not acking\n");
//                            // send(client_sock, "\nwait for..", 8, 0);
//                             CmdEnroll = IDLEENROL;

//                             const char *ack_message = "NETO";// nack for time out
//                             int err = send(client_sock, ack_message, strlen(ack_message), 0);
//                             if (err < 0) {
//                                 //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
//                             } else {
//                                 ESP_LOGI(TAGSOCKET, "back to idle mode\n");
//                                 cmd=false;
//                             }
//                             printf("\ncmd enroll flag status %d",CmdEnroll);
//                             vTaskDelay(10);

//                             }
//                         }

//                     }
//                 }else{ 

//                     ESP_LOGE(TAGSOCKET, "invalid %d", errno);

//                     }
//                     // if (total_received >= ACK_SIZE) {
//                     //     const char *ack_message = "ACK";
//                     //     int err = send(client_sock, ack_message, strlen(ack_message), 0);
//                     //     if (err < 0) {
//                     //         ESP_LOGE(TAGSOCKET, "Error sending ACK: errno %d", errno);
//                     //     } else {
//                     //         ESP_LOGI(TAGSOCKET, "ACK sent to client\n");
//                     //     }
//                     //     total_received = 0; // Reset the counter after sending ACK
//                     // }

//             }

//     }
// }
