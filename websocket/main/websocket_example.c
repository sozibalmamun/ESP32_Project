#include "websocket_example.h"

const char *TAG = "WEBSOCKET";
const char *TAGSTOMP = "STOMP_CLIENT";



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

            if((char *)data->data_ptr[0]=='o'){
            stomp_client_connect();            
            }  
            else if((char *)data->data_ptr[0]=='a'){
                stomp_client_handle_message(&data->data_ptr[3]);
            }else if((char *)data->data_ptr[0]=='h'){
                ESP_LOGI(TAG, "Ping");
                if(!stompSend(testdata,"/app/cloud")){
                ESP_LOGI(TAGSTOMP, "Data sending error");

                }
            }else if((char *)data->data_ptr[0]=='c'){

                ESP_LOGW(TAG, "Received= %s",(char *)data->data_ptr);
                websocket_app_start();
            }
        }
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}



void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    websocket_app_start();
}


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

static void websocket_app_start(void)
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
        
            if(!stompSend(testdata,"/app/cloud"))ESP_LOGI(TAGSTOMP, "Data sending error");

        // Handle the received message
    } else if (strstr(message, "ERROR")) {

        ESP_LOGE(TAGSTOMP, "STOMP ERROR: %s", message);

    }
}