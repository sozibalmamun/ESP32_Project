#include "StomeClient.h"
#include "timeLib.h"
#include <math.h>


#define     TAG             "WEBSOCKET"
#define     TAGSTOMP        "STOMP_CLIENT"


// extern volatile uint8_t CmdEnroll;
// extern char personName[20];
// extern uint16_t personId;

// volatile uint8_t  CmdEnroll=IDLEENROL;
// char personName[20];
// uint16_t personId;


// char tcpBuffer[2024]; // Adjust MAX_TRANSACTION_SIZE as needed
// TickType_t erolTimeOut;



void stomp_client_connect() {

    char connect_frame[100] = "[\"CONNECT\\naccept-version:1.1\\nhost:grozziieget.zjweiting.com\\n\\n\\u0000\"]";
    if(!esp_websocket_client_send_text(client, connect_frame, strlen(connect_frame), portMAX_DELAY))
    ESP_LOGI(TAGSTOMP, "Fail Connect pac");

}


void stomp_client_subscribe(char* topic) {
    
    char connect_frame[100] ;
    // snprintf(connect_frame, sizeof(connect_frame), "[\"SUBSCRIBE\\nid:sub-0\\ndestination:%s\\nack:client\\n\\n\\u0000\"]", topic);//done
    snprintf(connect_frame, sizeof(connect_frame), "[\"SUBSCRIBE\\nid:sub-0\\ndestination:%s\\nack:auto\\n\\n\\u0000\"]", topic);

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


// bool stompSend(char * buff, char* topic){

//     char tempFrame[CHANK_SIZE+1]; 
//     memset(tempFrame,0,sizeof(tempFrame));

//     uint16_t currentIndex=0;
//     uint16_t buffLen =strlen(buff);
//     ESP_LOGI(TAGSTOMP, "Sending  total chank :%d\n", (int)ceil(buffLen/CHANK_SIZE));
//     do{
//         memset(tempFrame,0,sizeof(tempFrame));
//         if(buffLen<=CHANK_SIZE){
//             currentIndex ? memcpy(&tempFrame,&buff[currentIndex-1],buffLen) : memcpy(&tempFrame,&buff[currentIndex],buffLen);
//             // memcpy(&tempFrame,&buff[currentIndex-1],buffLen);
//             buffLen= buffLen - buffLen;
//             ESP_LOGI(TAGSTOMP, "Sending last Chank\n");

//         }else{

//             currentIndex ? memcpy(&tempFrame,&buff[currentIndex-1],sizeof(tempFrame)-1) : memcpy(&tempFrame,&buff[currentIndex],sizeof(tempFrame)-1);
//             // memcpy(&tempFrame,&buff[currentIndex-1],sizeof(tempFrame));
//             // currentIndex+= CHANK_SIZE;
//             // buffLen= buffLen - CHANK_SIZE;
//         }
//         tempFrame[strlen(tempFrame)] = '\0';  // Null-terminate the chunk


//         char connect_frame[strlen(tempFrame)+37+strlen(topic)];
//         memset(connect_frame,0,sizeof(connect_frame));

//         // ESP_LOGI(TAGSTOMP, "Sending  tempFrame len :%d dynamic pac len %d\n", strlen(tempFrame) ,sizeof(connect_frame));

//         snprintf(connect_frame, sizeof(connect_frame), "[\"SEND\\ndestination:%s\\n\\n=%s=\\n\\n\\u0000\"]", topic, tempFrame);

//         ESP_LOGI(TAGSTOMP, "Sending STOMP MSG :\n%s", connect_frame);

//         if(!esp_websocket_client_is_connected(client)){

//             ESP_LOGE(TAGSTOMP, "Stomp disconnect\n");
//             wifiStatus=0x01;
//             stomp_client_connect(); 

//             return false;//
//         }
//         if(esp_websocket_client_send_text(client, connect_frame, strlen(connect_frame), portMAX_DELAY)!=ESP_OK){

//             // ESP_LOGI(TAGSTOMP, "Sending STOMP   sent len :%d  remain   %d\n", currentIndex,buffLen);

//             currentIndex+= CHANK_SIZE;

//             if(buffLen>0)buffLen= buffLen - CHANK_SIZE; // check bufflen 0 or not then calculate 

//         }else {

//             ESP_LOGI(TAGSTOMP, "Sending STOMP FAIL");
//             return false;

//         }

//     }while(buffLen!=0);


// return true;

// }

bool stompSend(char * buff, char* topic){

    char tempFrame[CHANK_SIZE+1]; 
    memset(tempFrame,0,sizeof(tempFrame));

    uint16_t currentIndex=0;
    uint16_t buffLen =strlen(buff);
    ESP_LOGW(TAGSTOMP, "Sending  total chank :%d\n", (int)ceil(buffLen/CHANK_SIZE)>1?(int)ceil(buffLen/CHANK_SIZE):1);
    do{
        memset(tempFrame,0,sizeof(tempFrame));
        if(buffLen<=CHANK_SIZE){
            currentIndex ? memcpy(&tempFrame,&buff[currentIndex-1],buffLen) : memcpy(&tempFrame,&buff[currentIndex],buffLen);
            buffLen= buffLen - buffLen;
            // ESP_LOGI(TAGSTOMP, "Sending last Chank\n");

        }else{

            currentIndex ? memcpy(&tempFrame,&buff[currentIndex-1],sizeof(tempFrame)-1) : memcpy(&tempFrame,&buff[currentIndex],sizeof(tempFrame)-1);

        }
        tempFrame[strlen(tempFrame)] = '\0';  // Null-terminate the chunk


        char sendingFrame[strlen(tempFrame)+37+strlen(topic)];
        memset(sendingFrame,0,sizeof(sendingFrame));

        snprintf(sendingFrame, sizeof(sendingFrame), "[\"SEND\\ndestination:%s\\n\\n%s\\n\\n\\u0000\"]", topic, tempFrame);

        ESP_LOGI(TAGSTOMP, "Sending STOMP MSG :\n%s", sendingFrame);

        if(!esp_websocket_client_is_connected(client)){

            ESP_LOGE(TAGSTOMP, "Stomp disconnect\n");
            wifiStatus=0x01;
            return false;//
        }
        if(esp_websocket_client_send_text(client, sendingFrame, strlen(sendingFrame), portMAX_DELAY)!=ESP_OK){

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


bool imagesent(uint8_t *buff, uint16_t buffLen, uint8_t h, uint8_t w ,char* name,uint16_t id, char* topic) {

    char tempFrame[(CHANK_SIZE * 2) + 1]; // +1 for null-terminator
    memset(tempFrame,0,sizeof(tempFrame));
    uint16_t currentIndex=0;
    ESP_LOGW(TAGSTOMP, "Sending len:%d total chank :%d \n",buffLen, (int)ceil(buffLen/CHANK_SIZE)+1);

    // sent image info
    char imageInfo[30];
    snprintf(imageInfo, sizeof(imageInfo), "%d %d %d %s %d",buffLen, h, w, name, id);
    stompSend(imageInfo,topic);


    do{

        // uint16_t chunkLen = (buffLen <= CHANK_SIZE) ? buffLen : CHANK_SIZE;
        memset(tempFrame,0,sizeof(tempFrame));
        if(buffLen<=CHANK_SIZE){
            // currentIndex ? memcpy(&tempFrame,&buff[currentIndex-1],buffLen) : memcpy(&tempFrame,&buff[currentIndex],buffLen);
        if(currentIndex){
            for (int i = 0; i < buffLen-1; i++) {
                sprintf(&tempFrame[i*2], "%02x", buff[(currentIndex-1) + i]);
            }
        }else{
            for (int i = 0; i < buffLen-1; i++) {
                sprintf(&tempFrame[i*2], "%02x", buff[currentIndex + i]);
            }
        }
            buffLen= buffLen - buffLen;
            ESP_LOGI(TAGSTOMP, "Sending last Chank\n");

        }else{
            // currentIndex ? memcpy(&tempFrame,&buff[currentIndex-1],sizeof(tempFrame)-1) : memcpy(&tempFrame,&buff[currentIndex],sizeof(tempFrame)-1);
        if(currentIndex){
            for (int i = 0; i < CHANK_SIZE-1; i++) {
                sprintf(&tempFrame[i*2], "%02x", buff[(currentIndex-1) + i]);
            }
        }else{
            for (int i = 0; i < CHANK_SIZE-1; i++) {
                sprintf(&tempFrame[i*2], "%02x", buff[currentIndex + i]);
            }
        }


        }
        tempFrame[sizeof(tempFrame)] = '\0';  // Null-terminate the chunk


        char connect_frame[sizeof(tempFrame)+37+strlen(topic)];
        memset(connect_frame,0,sizeof(connect_frame));

        // ESP_LOGI(TAGSTOMP, "Sending  tempFrame len :%d dynamic pack len %d\n", strlen(tempFrame) ,sizeof(connect_frame));

        snprintf(connect_frame, sizeof(connect_frame), "[\"SEND\\ndestination:%s\\n\\n%s\\n\\n\\u0000\"]", topic, tempFrame);

        ESP_LOGI(TAGSTOMP, "Sending STOMP MSG :\n%s", connect_frame);

        if(!esp_websocket_client_is_connected(client)){

            ESP_LOGE(TAGSTOMP, "Stomp disconnect\n");
            wifiStatus=0x01;
            stomp_client_connect(); 

            return false;//
        }
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
                //---------------------------------------------------------------

                time_library_time_t current_time;
                uint8_t clockType = get_time(&current_time, 1);
                char tempFrame[27] ;
                snprintf(tempFrame, sizeof(tempFrame), "%d %d %d %d %d %d %s",current_time.year,current_time.month,current_time.day,current_time.hour,current_time.minute,current_time.second,
                day_names[calculate_day_of_week( current_time.year, current_time.month, current_time.day )]);

                if(!stompSend(tempFrame, PUBLISH_TOPIC)){
                ESP_LOGI(TAGSTOMP, "sending error");
                }
                //--------------------------------------------------------------
            }else if(data->data_ptr[0]=='c'){

                // ESP_LOGW(TAG, "Received= %s",(char *)data->data_ptr);
                wifiStatus=0x01;
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
    // websocket_cfg.task_stack = 8192;  // Increased stack size
    websocket_cfg.task_prio = 5;      // Set an appropriate task priority


    // ESP_LOGI(TAG, "Constructed WebSocket URL: %s", websocket_cfg.uri);
    // ESP_LOGI(TAG, "Constructed WebSocket PATH: %s", websocket_cfg.path);


    // ESP_LOGI(TAG, "Initializing global CA store...");
    ESP_ERROR_CHECK(esp_tls_set_global_ca_store((const unsigned char *)echo_org_ssl_ca_cert, sizeof(echo_org_ssl_ca_cert)));


    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);
}
