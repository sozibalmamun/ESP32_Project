#include "StomeClient.h"
#include "timeLib.h"
#include <math.h>
#include "esp_wifi.h"


//----------------------


#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
typedef struct {
    char* data;
    char* topic;
    char* fileName;
} StompMessage;
QueueHandle_t stompQueue;

#define MAX_QUEUE_SIZE 10


// uint
//-----------------------



#define     TAG             "WSS"
#define     TAGSTOMP        "STOMP_CLIENT"

int8_t percentage=0;


static bool is_wifi_connected() {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        return true;
    }
    ESP_LOGI(TAG, "WiFi disconnected");
    return false;
}

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
        
        networkStatus=STOMP_CONNECTED;
        ESP_LOGI(TAGSTOMP, " STOMP Subscribed");
        initStompSender();
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

    char tempFrame[CHUNK_SIZE+1]; 
    memset(tempFrame,0,sizeof(tempFrame));

    uint16_t currentIndex=0;
    uint16_t buffLen =strlen(buff);
    // ESP_LOGW(TAGSTOMP, "Sending  total len: %d chank: %d\n", buffLen, (int)ceil(buffLen/CHUNK_SIZE)>1?(int)ceil(buffLen/CHUNK_SIZE):1);

    do{
        memset(tempFrame,0,sizeof(tempFrame));
        if(buffLen<=CHUNK_SIZE){
            currentIndex ? memcpy(&tempFrame,&buff[currentIndex-1],buffLen) : memcpy(&tempFrame,&buff[currentIndex],buffLen);
            buffLen= buffLen - buffLen;
            // ESP_LOGI(TAGSTOMP, "Sending last Chank\n");

        }else{

            currentIndex ? memcpy(&tempFrame,&buff[currentIndex-1],sizeof(tempFrame)-1) : memcpy(&tempFrame,&buff[currentIndex],sizeof(tempFrame)-1);

        }
        tempFrame[strlen(tempFrame)] = '\0';  // Null-terminate the chunk


        char sendingFrame[strlen(tempFrame)+47+strlen(topic)];
        memset(sendingFrame,0,sizeof(sendingFrame));

        strcat(sendingFrame, "[\"SEND\\ndestination:");
        strcat(sendingFrame, topic);
        strcat(sendingFrame, "\\n\\n");
        strcat(sendingFrame, tempFrame);
        strcat(sendingFrame, "\\n\\n\\u0000\"]");


        // ESP_LOGI(TAGSTOMP, "Sending STOMP MSG :\n%s", sendingFrame);

        if(networkStatus != STOMP_CONNECTED){
            ESP_LOGE(TAGSTOMP, "Stomp disconnect\n");
            if(networkStatus>WIFI_CONNECTED)networkStatus = WSS_CONNECTED;   
            vTaskDelay(100);  
            continue; // Retry sending

        }

        if(esp_websocket_client_send_text(client, sendingFrame, strlen(sendingFrame), portMAX_DELAY)==ESP_OK){

            ESP_LOGI(TAGSTOMP, "Sending STOMP FAIL");
            if(networkStatus>WIFI_CONNECTED)networkStatus = WSS_CONNECTED;   
            vTaskDelay(100); 

            continue; // Retry sending

        }else {
            // ESP_LOGI(TAGSTOMP, "Sending STOMP   sent len :%d  remain   %d\n", currentIndex,buffLen);
            currentIndex+= CHUNK_SIZE;
            if(buffLen>0)buffLen= buffLen - CHUNK_SIZE; // check bufflen 0 or not then calculate 
            vTaskDelay(30);
        }

    }while(buffLen!=0);

return true;
}


// bool stompSend(char * buff, char* topic){

//     uint16_t currentIndex = 0;
//     uint16_t buffLen = strlen(buff);
//     char *chunk;

//     // ESP_LOGW(TAGSTOMP, "Sending  total len: %d chank: %d\n", buffLen, (int)ceil(buffLen/CHANK_SIZE)>1?(int)ceil(buffLen/CHANK_SIZE):1);

//     while (buffLen > 0) {
    
//         // Allocate memory for the chunk to send
//         chunk = malloc(CHUNK_SIZE + 1);
//         if (chunk == NULL) {
//             ESP_LOGE(TAGSTOMP, "Failed to allocate memory for message chunk");
//             return false;
//         }
//         // Copy data into the chunk
//         memset(chunk, 0, CHUNK_SIZE + 1);
//         uint16_t chunkSize = (buffLen < CHUNK_SIZE) ? buffLen : CHUNK_SIZE;
//         memcpy(chunk, &buff[currentIndex], chunkSize);


//         char sendingFrame[CHUNK_SIZE+47+strlen(topic)];
//         memset(sendingFrame,0,sizeof(sendingFrame));

//         strcat(sendingFrame, "[\"SEND\\ndestination:");
//         strcat(sendingFrame, topic);
//         strcat(sendingFrame, "\\n\\n");
//         strcat(sendingFrame, chunk);
//         strcat(sendingFrame, "\\n\\n\\u0000\"]");
//         free(chunk); // Free allocated memory on failure

//         // ESP_LOGI(TAGSTOMP, "Sending STOMP MSG :\n%s", sendingFrame);

//         if(networkStatus != STOMP_CONNECTED){
//             ESP_LOGE(TAGSTOMP, "Stomp disconnect\n");
//             if(networkStatus>WIFI_CONNECTED)networkStatus = WSS_CONNECTED;   
//             vTaskDelay(100);  
//             continue; // Retry sending

//         }

//         if(esp_websocket_client_send_text(client, sendingFrame, strlen(sendingFrame), portMAX_DELAY)==ESP_OK){

//             ESP_LOGI(TAGSTOMP, "Sending STOMP FAIL");
//             if(networkStatus>WIFI_CONNECTED)networkStatus = WSS_CONNECTED;   
//             vTaskDelay(100); 

//             continue; // Retry sending

//         }else {
//             // ESP_LOGI(TAGSTOMP, "Sending STOMP   sent len :%d  remain   %d\n", currentIndex,buffLen);
//         // Update the current index and remaining buffer length
//         currentIndex += chunkSize;
//         buffLen -= chunkSize;
//         }

//     }

// return true;
// }



bool imagesent(uint8_t* buff, uint16_t buffLen, uint8_t h, uint8_t w, char* name, uint16_t id, char* topic) {
    // Calculate the required size for hex string
    size_t tempLen = buffLen * 2 + 1; // 2 characters per byte + null terminator
    char* hexString =  (char *)heap_caps_malloc(tempLen, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (hexString == NULL) {
        ESP_LOGE(TAGSTOMP, "Memory allocation for hexString failed");
        return false;
    }

    // Convert the entire buffer to a hex string
    for (uint16_t i = 0; i < buffLen; i++) {
        sprintf(&hexString[i * 2], "%02x", buff[i]);
    }

    // Null-terminate the hex string
    hexString[tempLen - 1] = '\0';

    uint16_t totalChank= (int)ceil((double)strlen(hexString) / IMAGE_CHANK_SIZE);


    ESP_LOGW(TAGSTOMP, "Total hex string length: %d, Chunks to send: %d\n", strlen(hexString), totalChank);




    // Send image info
    char imageInfo[35];
    snprintf(imageInfo, sizeof(imageInfo), "%d %d %d %s %d %d", buffLen, w, h, name, id,totalChank);
    if (!stompSend(imageInfo, topic)) {
        heap_caps_free(hexString);
        return false;
    }
    vTaskDelay(50);

    // Send the hex string in chunks
    uint16_t currentIndex = 0;
    uint16_t chankNo=0;
    while (currentIndex < strlen(hexString)) {
        char chunk[IMAGE_CHANK_SIZE + 1]; // Buffer for each chunk
        memset(chunk, 0, sizeof(chunk));

        // Calculate remaining length and copy chunk
        size_t chunkLen = strlen(hexString) - currentIndex;
        if (chunkLen > IMAGE_CHANK_SIZE) {
            chunkLen = IMAGE_CHANK_SIZE;
        }

        strncpy(chunk, &hexString[currentIndex], chunkLen);

        // Prepare the STOMP frame to send
        char sentFrame[sizeof(chunk) + 55 + strlen(topic)];
        memset(sentFrame, 0, sizeof(sentFrame));

        snprintf(sentFrame, sizeof(sentFrame), "[\"SEND\\ndestination:%s\\n\\n%d %d %s\\n\\n\\u0000\"]", topic, id ,chankNo+1,chunk);

        // printf("Chunk No: %d\n", chankNo+1);// chank no 


        if (networkStatus != STOMP_CONNECTED) {
            ESP_LOGE(TAGSTOMP, "Stomp disconnected\n");
            vTaskDelay(50);
            if(networkStatus>WIFI_CONNECTED)networkStatus = WSS_CONNECTED;   
            vTaskDelay(100);
            // free(hexString);
            continue; // Retry sending
        }

        if(is_wifi_connected()){

            if (esp_websocket_client_send_text(client, sentFrame, strlen(sentFrame), portMAX_DELAY) == 0) {
                vTaskDelay(50);
                ESP_LOGI(TAGSTOMP, "STOMP send failed. Retrying...\n");
                if(networkStatus>WIFI_CONNECTED)networkStatus = WSS_CONNECTED;   
                vTaskDelay(100);
                continue; // Retry sending

            }else{
                // vTaskDelay(2);
                currentIndex += chunkLen;
                chankNo++;// no of chank 
                float percentage_float = (chankNo /(float)totalChank) * 100;
                percentage = (int)percentage_float;
                
                // printf("total chank  %d send %d percentage: %d\n",totalChank,  chankNo,percentage);
                if(percentage>=100)percentage=0;
            }
        }else networkStatus = WIFI_DISS;

    }
    heap_caps_free(hexString);
    return true;
}




static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        // ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        networkStatus=WSS_CONNECTED;

        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");

        networkStatus=WIFI_CONNECTED;


        break;
    case WEBSOCKET_EVENT_DATA:

        if (data->op_code == 0x08 && data->data_len == 2) {
            ESP_LOGW(TAG, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);

            vTaskDelay(100);
        } else {

            if(data->data_ptr[0]=='o')stomp_client_connect();   

            else if(data->data_ptr[0]=='a'){

                // ESP_LOGW(TAG, "Received= %s",(char *)data->data_ptr);
                stomp_client_handle_message(&data->data_ptr[3]);

            }

            else if(data->data_ptr[0]=='h'){
                //---------------------------------------------------------------
                if( networkStatus==STOMP_CONNECTED){
                    ESP_LOGI(TAG, "Ping");
                    time_library_time_t current_time;
                    uint8_t clockType = get_time(&current_time, 1);
                    char tempFrame[27] ;
                    snprintf(tempFrame, sizeof(tempFrame), "%d %d %d %d %d %d %s",current_time.year,current_time.month,current_time.day,current_time.hour,current_time.minute,current_time.second,
                    day_names[calculate_day_of_week( current_time.year, current_time.month, current_time.day )]);

                    if(!stompSend(tempFrame, PUBLISH_TOPIC)){
                        // ESP_LOGI(TAGSTOMP, "sending error");

                    }

                }
                //--------------------------------------------------------------
            }else if(data->data_ptr[0]=='c'){

                // ESP_LOGW(TAG, "Received= %s",(char *)data->data_ptr);
                networkStatus=WIFI_CONNECTED;
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

//---------------------------rtoss rnd---------------------------------------------

static void stompSenderTask(void *pvParameters) {
    StompMessage msg;
    char sendingFrame[CHUNK_SIZE + 47 + 64]; // Adjust size based on topic and chunk size

    while (true) {
        // Wait for a message from the queue
        if (xQueueReceive(stompQueue, &msg, portMAX_DELAY) == pdTRUE) {
            // Construct the sending frame
            snprintf(sendingFrame, sizeof(sendingFrame), "[\"SEND\\ndestination:%s\\n\\n%s\\n\\n\\u0000\"]", msg.topic, msg.data);

            ESP_LOGW(TAGSTOMP, " msg.topic: %s msg.data: %s\n", msg.topic, msg.data);

            // Ensure network status is connected
            if (networkStatus != STOMP_CONNECTED) {
                ESP_LOGE(TAGSTOMP, "Stomp disconnected, cannot send message\n");
                if (networkStatus > WIFI_CONNECTED) {
                    networkStatus = WSS_CONNECTED;
                }
                vTaskDelay(pdMS_TO_TICKS(100)); // Delay before retrying
                continue;
            }

            // Send the message using WebSocket
            if (esp_websocket_client_send_text(client, sendingFrame, strlen(sendingFrame), portMAX_DELAY) == 0) {
                ESP_LOGI(TAGSTOMP, "Sending STOMP message failed");
                if (networkStatus > WIFI_CONNECTED) {
                    networkStatus = WSS_CONNECTED;
                }
                vTaskDelay(pdMS_TO_TICKS(100)); // Delay before retrying
            } else {
                ESP_LOGI(TAGSTOMP, "STOMP message sent successfully");
                if (msg.fileName != NULL) {
                    ESP_LOGI(TAGSTOMP, "Deleting file: %s", msg.fileName);
                    delete_file(msg.fileName); // Delete the file after sending
                }
            }

            // Free the allocated memory for the message data
            heap_caps_free(msg.data);             
        }
    }
}

bool logSend(char *buff, char *fsFileName, char *topic) {
    uint16_t currentIndex = 0;
    uint16_t buffLen = strlen(buff);
    uint16_t totalChunks = (buffLen + CHUNK_SIZE - 1) / CHUNK_SIZE; // Calculate total chunks

    ESP_LOGW(TAGSTOMP, "Total length: %d, Chunks to send: %d, File name: %s\n", buffLen, totalChunks, fsFileName);

    while (buffLen > 0) {
        // Allocate memory for the chunk to send
        char *chunk = (char *)heap_caps_malloc(CHUNK_SIZE + 1, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        if (chunk == NULL) {
            ESP_LOGE(TAGSTOMP, "Failed to allocate memory for message chunk");
            return false;
        }

        // Copy data into the chunk
        memset(chunk, 0, CHUNK_SIZE + 1);
        uint16_t chunkSize = (buffLen < CHUNK_SIZE) ? buffLen : CHUNK_SIZE;
        memcpy(chunk, &buff[currentIndex], chunkSize);

        // Prepare the message structure
        StompMessage msg = {
            .data = chunk,
            .topic = topic,
            .fileName = NULL,
        };

        // Check if this is the last chunk
        if (chunkSize < CHUNK_SIZE) {

            memset(chunk, 0, CHUNK_SIZE + 1);
            memcpy(chunk, fsFileName, strlen(fsFileName));

            msg.fileName = chunk;
            ESP_LOGI(TAGSTOMP, "Last chunk with file name: %s", msg.fileName);
        }

        // Send the message to the queue
        if (xQueueSend(stompQueue, &msg, portMAX_DELAY) != pdTRUE) {
            ESP_LOGE(TAGSTOMP, "Failed to queue message for sending");
            heap_caps_free(chunk); // Free allocated memory on failure     
            continue;
        } else {
            currentIndex += chunkSize;
            buffLen -= chunkSize;
            ESP_LOGI(TAGSTOMP, "Queued message for sending");
        }
    }

    return true;
}




// Initialization function to create the queue and sender task
void initStompSender() {
    // Create the queue for STOMP messages
    stompQueue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(StompMessage));
    if (stompQueue == NULL) {
        ESP_LOGE(TAGSTOMP, "Failed to create stomp message queue");
        return;
    }else{
        ESP_LOGW(TAGSTOMP, "create stomp message queue");

    }

    // Create the sender task
    xTaskCreatePinnedToCore(stompSenderTask, "StompSenderTask", 6*1024, NULL, 3,NULL, 1);

}