#include "StomeClient.h"



#define     TAG             "WEBSOCKET"
#define     TAGSTOMP        "STOMP_CLIENT"
#define     TAG_WI_FI       "Wifi Debug"
#define     TAG_FS          "FS Debug"



const char *ssid = "myssid";
const char *pass = "mypassword";

// extern volatile uint8_t CmdEnroll;
// extern char personName[20];
// extern uint16_t personId;

// volatile uint8_t  CmdEnroll=IDLEENROL;
// char personName[20];
// uint16_t personId;


// char tcpBuffer[2024]; // Adjust MAX_TRANSACTION_SIZE as needed
// TickType_t erolTimeOut;


static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_id == WIFI_EVENT_STA_START) {
        printf("WIFI CONNECTING....\n");
    } else if (event_id == WIFI_EVENT_STA_CONNECTED) {

        printf("WiFi CONNECTED\n");
         wifiStatus=0x01;

        vTaskDelay(500);
        stompAppStart();

    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {

        // printf("WiFi lost connection\n");
         wifiStatus=0x00;
        esp_websocket_client_stop( client);
        esp_wifi_connect();
        printf("Retrying to Connect...\n");
    } else if (event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG_WI_FI, "WiFi got IP: " IPSTR, IP2STR(&event->ip_info.ip));

        // uint8_t mac[6];
        // esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
        // ESP_LOGI(TAG_WI_FI, "MAC address: %02x:%02x:%02x:%02x:%02x:%02x",
        //          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

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
    
    // set_and_print_hostname(DEVICE_NAME);

    // Wi-Fi Start Phase
    esp_wifi_start();
    esp_wifi_set_mode(WIFI_MODE_STA);

    // Wi-Fi Connect Phase
    esp_wifi_connect();

}



// void print_hostname() {
//     esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
//     if (netif == NULL) {
//         ESP_LOGE(TAG_WI_FI, "Failed to get netif handle");
//         return;
//     }

//     const char *hostname;
//     esp_err_t err = esp_netif_get_hostname(netif, &hostname);
//     if (err == ESP_OK) {
//         ESP_LOGI(TAG_WI_FI, "Hostname: %s", hostname);
//     } else {
//         ESP_LOGE(TAG_WI_FI, "Failed to get hostname: %s", esp_err_to_name(err));
//     }
// }
// Function to set and print the hostname
// void set_and_print_hostname(char *hostName) {

//     esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
//     if (netif == NULL) {
//         ESP_LOGE(TAG_WI_FI, "Failed to get netif handle");
//         return;
//     }

//     // Set the hostname to "THTFace"
//     const char *new_hostname = hostName;
//     esp_err_t err = esp_netif_set_hostname(netif, new_hostname);
//     if (err == ESP_OK) {
//         ESP_LOGI(TAG_WI_FI, "Hostname set to: %s", new_hostname);
//     } else {
//         ESP_LOGE(TAG_WI_FI, "Failed to set hostname: %s", esp_err_to_name(err));
//         return;
//     }

//     // Retrieve and log the hostname to verify
//     const char *hostname;
//     err = esp_netif_get_hostname(netif, &hostname);
//     if (err == ESP_OK) {
//         ESP_LOGI(TAG_WI_FI, "Hostname: %s", hostname);
//     } else {
//         ESP_LOGE(TAG_WI_FI, "Failed to get hostname: %s", esp_err_to_name(err));
//     }
// }

void stomp_client_connect() {

    char connect_frame[100] = "[\"CONNECT\\naccept-version:1.1\\nhost:grozziieget.zjweiting.com\\n\\n\\u0000\"]";
    esp_websocket_client_send_text(client, connect_frame, strlen(connect_frame), portMAX_DELAY);
}


void stomp_client_subscribe(char* topic) {
    
    char connect_frame[100] ;
    snprintf(connect_frame, sizeof(connect_frame), "[\"SUBSCRIBE\\nid:sub-0\\ndestination:%s\\nack:client\\n\\n\\u0000\"]", topic);
    if(esp_websocket_client_send_text(client, connect_frame, strlen(connect_frame), portMAX_DELAY)>0){
        
        wifiStatus=0x02;
        ESP_LOGI(TAGSTOMP, " STOMP Subscribed");
    }

}
void stomeAck(const char * message){
    char output[20] ;
    const char *jsonStart = strstr(message, "message-id:");
    if (jsonStart) {
        jsonStart += strlen("message-id:"); // Move past the starting point
        // Locate the end of the message within the JSON payload
        const char *jsonEnd = strstr(jsonStart, "-");
        if (jsonEnd) {
            // Copy the message content into the output buffer
            size_t length = jsonEnd - jsonStart;
            strncpy(output, jsonStart, length);
            output[length] = '\0'; // Null-terminate the output string
        }
    }
    char connect_frame[100] ;
    snprintf(connect_frame, sizeof(connect_frame), "[\"ACK\\nsubscription:1\\nmessage-id:%s\\ntransaction:tx1\\n\\n\\u0000\"]",output);
    ESP_LOGI(TAGSTOMP, "STOMP ACKING %s\n", connect_frame);
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
            // currentIndex+= CHANK_SIZE;
            // buffLen= buffLen - CHANK_SIZE;
        }
        tempFrame[strlen(tempFrame)] = '\0';  // Null-terminate the chunk


        char connect_frame[strlen(tempFrame)+37+strlen(topic)];memset(connect_frame,0,sizeof(connect_frame));

        // ESP_LOGI(TAGSTOMP, "Sending  tempFrame len :%d dynamic pac len %d\n", strlen(tempFrame) ,sizeof(connect_frame));

        snprintf(connect_frame, sizeof(connect_frame), "[\"SEND\\ndestination:%s\\n\\n%s\\n\\n\\u0000\"]", topic, tempFrame);
        // ESP_LOGI(TAGSTOMP, "Sending  connect_frame len :%d\n", strlen(connect_frame));

        // ESP_LOGI(TAGSTOMP, "Sending STOMP MSG :\n%s", connect_frame);

        if(!esp_websocket_client_is_connected(client))return false;

        if(esp_websocket_client_send_text(client, connect_frame, strlen(connect_frame), portMAX_DELAY)!=ESP_OK){
            // ESP_LOGI(TAGSTOMP, "Sending STOMP   sent len :%d  remain   %d\n", currentIndex,buffLen);

            currentIndex+= CHANK_SIZE;

            if(buffLen>0)buffLen= buffLen - CHANK_SIZE; // check bufflen 0 or not then calculate 

        }else {

            ESP_LOGI(TAGSTOMP, "Sending STOMP FAIL");
            return false;

        }

    }while(buffLen!=0);


return true;

}


static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        // ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");

        break;
    case WEBSOCKET_EVENT_DATA:

        if (data->op_code == 0x08 && data->data_len == 2) {
            ESP_LOGW(TAG, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);
        } else {

            if(data->data_ptr[0]=='o')stomp_client_connect();   

            else if(data->data_ptr[0]=='a'){

                // ESP_LOGW(TAG, "Received= %s",(char *)data->data_ptr);
                stomp_client_handle_message(&data->data_ptr[3]);

            }

            else if(data->data_ptr[0]=='h'){
                ESP_LOGI(TAG, "Ping");

                // if(!stompSend("testdata", PUBLISH_TOPIC)){
                // ESP_LOGI(TAGSTOMP, "Data sending error");
                // }

            }else if(data->data_ptr[0]=='c'){

                ESP_LOGW(TAG, "Received= %s",(char *)data->data_ptr);
                stompAppStart();
            }
            // ESP_LOGI(TAG, "WEBSOCKET_free");
            memset(data->data_ptr,0,data->data_len);
        }
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}
void stomp_client_handle_message( const char *message) {

    // ESP_LOGI(TAGSTOMP, "Received STOMP message:\n%s", message);
        if (strstr(message, "CONNECTED")) {
        // ESP_LOGI(TAGSTOMP, "STOMP CONNECTED");
        // Subscribe to a topic
        stomp_client_subscribe(SUBCRIBE_TOPIC);
    } else if (strstr(message, "MESSAGE")) {

        // stomeAck(message);

        // if(!stompSend(testdata,"/app/cloud"))ESP_LOGI(TAGSTOMP, "Data sending error");
        dataHandele(message);

        // Handle the received message
    } else if (strstr(message, "ERROR")) {

        ESP_LOGE(TAGSTOMP, "STOMP ERROR: %s", message);

    }

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
void stomp_client_int( stompInfo_cfg_t stompSetup ) {
   
    esp_websocket_client_config_t websocket_cfg = {};

    char socket[100];
    snprintf(socket, sizeof(socket), "%s", stompSetup.uri);

    int random1 = esp_random() % 999; // Generates a random number between 0 and 999
    int random2 = esp_random() % 999999; // Generates a random number between 0 and 999999
    snprintf(socket + strlen(socket), sizeof(socket) - strlen(socket), "%d/%d/websocket", random1, random2);

    websocket_cfg.uri = (const char*)socket;
    websocket_cfg.cert_pem = echo_org_ssl_ca_cert;   
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


    // ESP_LOGI(TAG, "Constructed WebSocket URL: %s", websocket_cfg.uri);
    // ESP_LOGI(TAG, "Constructed WebSocket PATH: %s", websocket_cfg.path);


    // ESP_LOGI(TAG, "Initializing global CA store...");
    ESP_ERROR_CHECK(esp_tls_set_global_ca_store((const unsigned char *)echo_org_ssl_ca_cert, sizeof(echo_org_ssl_ca_cert)));


    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);
}


uint64_t generate_unique_id(void)
{

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    uint64_t unique_id = ((uint64_t)mac[0] << 40) | ((uint64_t)mac[1] << 32) | ((uint64_t)mac[2] << 24) |

                         ((uint64_t)mac[3] << 16) | ((uint64_t)mac[4] << 8) | mac[5];

    return unique_id;

}
uint8_t get_wifi_signal_strength() {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        int32_t rssi = ap_info.rssi;
        if (rssi <= -100) {
            return 0;
        } else if (rssi >= 0) {
            return 100;
        } else {
            return (uint8_t)((rssi + 100) * 100 / 100);
        }
    } else {
        ESP_LOGE(TAG, "Failed to get Wi-Fi RSSI");
        return 0;
    }
}