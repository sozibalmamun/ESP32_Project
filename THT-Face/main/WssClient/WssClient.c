#include "WssClient.h"
#include "timeLib.h"
#include <math.h>

#define     TAG             "WSS"
#define     TAGSTOMP        "STOMP_CLIENT"

int8_t percentage=0;
int8_t maxTry=0;

void stomp_client_connect() {

    char connect_frame[100] = "[\"CONNECT\\naccept-version:1.1\\nhost:grozziieget.zjweiting.com\\n\\n\\u0000\"]";
    if(!esp_websocket_client_send_text(client, connect_frame, strlen(connect_frame), portMAX_DELAY));
    // ESP_LOGI(TAGSTOMP, "Fail Connect pac");

}


void stomp_client_subscribe(char* topic) {
    
    char connect_frame[100] ;
    // snprintf(connect_frame, sizeof(connect_frame), "[\"SUBSCRIBE\\nid:sub-0\\ndestination:%s\\nack:client\\n\\n\\u0000\"]", topic);//done
    snprintf(connect_frame, sizeof(connect_frame), "[\"SUBSCRIBE\\nid:sub-0\\ndestination:%s\\nack:auto\\n\\n\\u0000\"]", topic);

    if(esp_websocket_client_send_text(client, connect_frame, strlen(connect_frame), portMAX_DELAY)>0){
        
        networkStatus=STOMP_CONNECTED;
        ESP_LOGI(TAGSTOMP, " STOMP Subscribed");
    }

}

// void stomeAck(const char * message){
//     char output[20] ;
//     const char *jsonStart = strstr(message, "message-id:");
//     if (jsonStart) {
//         jsonStart += strlen("message-id:"); // Move past the starting point
//         // Locate the end of the message within the JSON payload
//         const char *jsonEnd = strstr(jsonStart, "-");
//         if (jsonEnd) {
//             // Copy the message content into the output buffer
//             size_t length = jsonEnd - jsonStart;
//             strncpy(output, jsonStart, length);
//             output[length] = '\0'; // Null-terminate the output string
//         }
//     }
//     char connect_frame[100] ;
//     snprintf(connect_frame, sizeof(connect_frame), "[\"ACK\\nsubscription:1\\nmessage-id:%s\\ntransaction:tx1\\n\\n\\u0000\"]",output);
//     // ESP_LOGI(TAGSTOMP, "STOMP ACKING %s\n", connect_frame);
//     esp_websocket_client_send_text(client, connect_frame, strlen(connect_frame), portMAX_DELAY);
// }





//=========================================done side====================================================================

bool stompSend(char *buff, char* topic) {
    char tempFrame[CHANK_SIZE + 1];
    memset(tempFrame, 0, sizeof(tempFrame));

    uint16_t currentIndex = 0;
    uint16_t buffLen = strlen(buff);

    ESP_LOGW(TAGSTOMP, "Total stomp length: %d data : %s\n", strlen(buff),buff );


    // Continue sending chunks while there is data left
    while (buffLen > 0) {
        memset(tempFrame, 0, sizeof(tempFrame));

        // Determine the length of the chunk to be sent
        size_t chunkLen = MIN(CHANK_SIZE, buffLen);

        // Copy the chunk to tempFrame
        memcpy(tempFrame, &buff[currentIndex], chunkLen);

        // Null-terminate the chunk
        tempFrame[chunkLen] = '\0';

        // Prepare the STOMP frame
        size_t sendingFrameLen = strlen(tempFrame) + 47 + strlen(topic);
        char* sendingFrame = (char*)heap_caps_malloc(sendingFrameLen + 1, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        if (sendingFrame == NULL) {
            // ESP_LOGE(TAGSTOMP, "Memory allocation for sendingFrame failed");
            return false;
        }

        memset(sendingFrame, 0, sendingFrameLen + 1);


        strcat(sendingFrame, "[\"SEND\\ndestination:");
        strcat(sendingFrame, topic);
        strcat(sendingFrame, "\\n\\n");

        strcat(sendingFrame, tempFrame);

        // sendingFrame[strlen(sendingFrame)] = 0x31; // Directly inserting a binary character at the correct location

        strcat(sendingFrame, "\\n\\n\\u0000\"]");

        ESP_LOGW(TAGSTOMP, "sending pac: %s\n", sendingFrame);

        // Check network status
        if (networkStatus != STOMP_CONNECTED) {
            ESP_LOGE(TAGSTOMP, "Stomp disconnected, retrying...");
            if (networkStatus > WIFI_CONNECTED) networkStatus = WSS_CONNECTED;
            heap_caps_free(sendingFrame);
            sendingFrame=NULL;
            vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay before retry
            maxTry++;
            if(maxTry>MAXTRY){
                maxTry=0;
                return false;
            }
            continue; // Retry sending
        }

        // Send the STOMP frame
        if (esp_websocket_client_send_text(client, sendingFrame, strlen(sendingFrame), portMAX_DELAY) == 0) {
            // ESP_LOGE(TAGSTOMP, "Sending STOMP failed, retrying...");
            if (networkStatus > WIFI_CONNECTED) networkStatus = WSS_CONNECTED;
            heap_caps_free(sendingFrame);
            sendingFrame=NULL;
            vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay before retry
            maxTry++;
            if(maxTry>MAXTRY){
                maxTry=0;
                return false;
            }
            continue; // Retry sending
        }

        // Update indices and lengths
        currentIndex += chunkLen;
        buffLen -= chunkLen;

        heap_caps_free(sendingFrame); // Free allocated memory for sendingFrame
        sendingFrame=NULL;
        // vTaskDelay(30 / portTICK_PERIOD_MS); // Delay to avoid flooding
    }

    return true;
}



// bool imagesent(uint8_t* buff, uint16_t buffLen, uint8_t h, uint8_t w, char* name, uint16_t id, char* topic) {
//     // Calculate the required size for hex string
//     size_t tempLen = buffLen * 2 + 1;
//     char* hexString = (char*)heap_caps_malloc(tempLen, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
//     if (hexString == NULL) {
//         // ESP_LOGE(TAGSTOMP, "Memory allocation for hexString failed");
//         return false;
//     }

//     // Convert buffer to hex string
//     for (uint16_t i = 0; i < buffLen; i++) {
//         sprintf(&hexString[i * 2], "%02x", buff[i]);
//     }
//     hexString[tempLen - 1] = '\0';

//     uint16_t totalChunks = (uint16_t)ceil((double)strlen(hexString) / IMAGE_CHANK_SIZE);
//     // ESP_LOGW(TAGSTOMP, "Total hex string length: %d, Chunks to send: %d\n", strlen(hexString), totalChunks);

//     // Send image info
//     char imageInfo[35];
//     snprintf(imageInfo, sizeof(imageInfo), "%d %d %d %s %d %d", buffLen, w, h, name, id, totalChunks);
//     if (!stompSend(imageInfo, topic)) {
//         heap_caps_free(hexString);
//         hexString=NULL;
//         return false;
//     }
//     vTaskDelay(50);

//     uint16_t currentIndex = 0;
//     uint16_t chunkNo = 0;

//     while (currentIndex < strlen(hexString)) {
//         size_t chunkLen = MIN(IMAGE_CHANK_SIZE, strlen(hexString) - currentIndex);
//         char* chunk = (char*)heap_caps_malloc(chunkLen + 1, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
//         if (chunk == NULL) {
//             // ESP_LOGE(TAGSTOMP, "Memory allocation for chunk failed");
//             heap_caps_free(hexString);
//             hexString=NULL;
//             return false;
//         }
//         strncpy(chunk, &hexString[currentIndex], chunkLen);
//         chunk[chunkLen] = '\0';

//     // Corrected length calculation for sentFrame
//         size_t sentFrameLen = chunkLen + 55; // Additional padding for format specifiers
//         char* sentFrame = (char*)heap_caps_malloc(sentFrameLen + 1, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
//         if (sentFrame == NULL) {
//             // ESP_LOGE(TAGSTOMP, "Memory allocation for sentFrame failed");
//             heap_caps_free(chunk);
//             heap_caps_free(hexString);
//             hexString=NULL;  chunk=NULL;

//             return false;
//         }
//         memset(sentFrame, 0, sentFrameLen + 1);
        // snprintf(sentFrame, sentFrameLen, "%d %d %s", id, chunkNo + 1, chunk);
//         printf("Chunk No: %d\n", chunkNo+1);// chank no 

//         vTaskDelay(10);

//         if (!stompSend(sentFrame, topic)) {

//             heap_caps_free(chunk);
//             heap_caps_free(hexString); // Free hex string memory
//             heap_caps_free(sentFrame);
//             hexString=NULL; chunk=NULL; sentFrame=NULL;
//             return false;

//         }

//         currentIndex += chunkLen;
//         chunkNo++;
//         float percentage_float = ((float)chunkNo / totalChunks) * 100;
//         percentage = (int)percentage_float;
//         if (percentage >= 100) percentage = 0;

//         heap_caps_free(chunk); // Free chunk memory
//         heap_caps_free(sentFrame);
//         chunk=NULL;sentFrame=NULL;

//     }

//     heap_caps_free(hexString); // Free hex string memory
//     hexString=NULL;  
//     return true;
// }

//====================================done end




bool stompS(uint8_t *buff, size_t buffLen) {

    uint8_t tempFrame[CHANK_SIZE + 1];
    memset(tempFrame, 0, sizeof(tempFrame));

    uint16_t currentIndex = 0;

    ESP_LOGW(TAGSTOMP, "Total stompS length: %d data : %s\n", buffLen,(char*)buff );



    while (buffLen > 0) {
        memset(tempFrame, 0, sizeof(tempFrame));

        size_t chunkLen = MIN(CHANK_SIZE, buffLen);
        memcpy(tempFrame, &buff[currentIndex], chunkLen);

        size_t sendingFrameLen = chunkLen + 100 ;  // Adjust size based on chunk size
        char* sendingFrame = (char*)heap_caps_malloc(sendingFrameLen + 1, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        if (sendingFrame == NULL) {
            return false;
        }
        memset(sendingFrame, 0, sendingFrameLen + 1);




        // char uinqeID[10];
        // snprintf(uinqeID, sizeof(uinqeID), "%08llu", generate_unique_id());
        // strcat(sendingFrame, DEVICE_VERSION_ID);
        // strcat(sendingFrame, uinqeID);
        // strcat(sendingFrame, " ");



        // strcat(sendingFrame, topic);
        // strcat(sendingFrame, "\\n\\n");
        // memcpy(&sendingFrame[strlen(sendingFrame)], tempFrame, chunkLen); 
        // strcat(sendingFrame, "\\n\\n\\u0000\"]");



        memcpy(&sendingFrame[strlen(sendingFrame)], tempFrame, chunkLen); 

        ESP_LOGW(TAGSTOMP, "stompS pac: %s\n", sendingFrame );


        // if (networkStatus != STOMP_CONNECTED) {
        //     if (networkStatus > WIFI_CONNECTED) networkStatus = WSS_CONNECTED;
        //     heap_caps_free(sendingFrame);
        //     vTaskDelay(1000 / portTICK_PERIOD_MS);
        //     maxTry++;
        //     if (maxTry > MAXTRY) {
        //         maxTry = 0;
        //         return false;
        //     }
        //     continue;
        // }

        // if (esp_websocket_client_send_text(client, sendingFrame, strlen(sendingFrame), portMAX_DELAY) == 0) {  esp_websocket_client_send_bin

        if (esp_websocket_client_send_bin(client, sendingFrame, strlen(sendingFrame), portMAX_DELAY) == 0) {  


            if (networkStatus > WIFI_CONNECTED) networkStatus = WSS_CONNECTED;
            heap_caps_free(sendingFrame);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            maxTry++;
            if (maxTry > MAXTRY) {
                maxTry = 0;
                return false;
            }
            continue;
        }

        currentIndex += chunkLen;
        buffLen -= chunkLen;

        heap_caps_free(sendingFrame);
    }

    return true;
}

bool imagesent(uint8_t* buff, uint16_t buffLen, uint8_t h, uint8_t w, char* name, uint16_t id, char* topic) {
    uint16_t totalChunks = (uint16_t)ceil((double)buffLen / IMAGE_CHANK_SIZE);

    // Send image info
    char imageInfo[35];
    snprintf(imageInfo, sizeof(imageInfo), "%d %d %d %s %d %d", buffLen, w, h, name, id, totalChunks);
    if (!stompS((uint8_t*)imageInfo, strlen(imageInfo))) {
        return false;
    }
    vTaskDelay(50);

    uint16_t currentIndex = 0;
    uint16_t chunkNo = 0;

    while (currentIndex < buffLen) {
        size_t chunkLen = MIN(IMAGE_CHANK_SIZE, buffLen - currentIndex);
        uint8_t* chunk = (uint8_t*)heap_caps_malloc(chunkLen, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        if (chunk == NULL) {
            return false;
        }
        memcpy(chunk, &buff[currentIndex], chunkLen);

        size_t sentFrameLen = chunkLen + 55; // Adjust for format specifiers
        char* sentFrame = (char*)heap_caps_malloc(sentFrameLen + 1, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        if (sentFrame == NULL) {
            heap_caps_free(chunk);
            return false;
        }
        memset(sentFrame, 0, sentFrameLen + 1);
        snprintf(sentFrame, sentFrameLen, "%d %d ", id, chunkNo + 1);
        memcpy(&sentFrame[strlen(sentFrame)], chunk, chunkLen);  // Copy binary chunk directly

        printf("Chunk No: %d\n", chunkNo + 1);  // Chunk number logging

        // vTaskDelay(10);

        if (!stompS((uint8_t*)sentFrame, strlen(sentFrame))) {
            heap_caps_free(chunk);
            heap_caps_free(sentFrame);
            return false;
        }

        currentIndex += chunkLen;
        chunkNo++;

        float percentage_float = ((float)chunkNo / totalChunks) * 100;
        percentage = (int)percentage_float;
        if (percentage >= 100) percentage = 0;

        heap_caps_free(chunk);
        heap_caps_free(sentFrame);
    }

    return true;
}









//--------------------backup

// bool stompSend(char * buff, char* topic){

//     char tempFrame[CHANK_SIZE+1]; 
//     memset(tempFrame,0,sizeof(tempFrame));

//     uint16_t currentIndex=0;
//     uint16_t buffLen =strlen(buff);
//     // ESP_LOGW(TAGSTOMP, "Sending  total len: %d chank: %d\n", buffLen, (int)ceil(buffLen/CHANK_SIZE)>1?(int)ceil(buffLen/CHANK_SIZE):1);

//     do{
//         memset(tempFrame,0,sizeof(tempFrame));
//         if(buffLen<=CHANK_SIZE){
//             currentIndex ? memcpy(&tempFrame,&buff[currentIndex-1],buffLen) : memcpy(&tempFrame,&buff[currentIndex],buffLen);
//             buffLen= buffLen - buffLen;
//             ESP_LOGI(TAGSTOMP, "Sending last Chank\n");

//         }else{

//             currentIndex ? memcpy(&tempFrame,&buff[currentIndex-1],sizeof(tempFrame)-1) : memcpy(&tempFrame,&buff[currentIndex],sizeof(tempFrame)-1);

//         }
//         tempFrame[strlen(tempFrame)] = '\0';  // Null-terminate the chunk


//         char sendingFrame[strlen(tempFrame)+47+strlen(topic)];
//         memset(sendingFrame,0,sizeof(sendingFrame));

//         strcat(sendingFrame, "[\"SEND\\ndestination:");
//         strcat(sendingFrame, topic);
//         strcat(sendingFrame, "\\n\\n");
//         strcat(sendingFrame, tempFrame);
//         strcat(sendingFrame, "\\n\\n\\u0000\"]");


//         // ESP_LOGI(TAGSTOMP, "Sending STOMP MSG :\n%s", sendingFrame);

//         if(networkStatus != STOMP_CONNECTED){
//             ESP_LOGE(TAGSTOMP, "Stomp disconnect\n");
//             if(networkStatus>WIFI_CONNECTED)networkStatus = WSS_CONNECTED;   
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//             continue; // Retry sending

//         }

//         if(esp_websocket_client_send_text(client, sendingFrame, strlen(sendingFrame), portMAX_DELAY)==ESP_OK){

//             ESP_LOGI(TAGSTOMP, "Sending STOMP FAIL");
//             if(networkStatus>WIFI_CONNECTED)networkStatus = WSS_CONNECTED;   
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//             continue; // Retry sending

//         }else {
//             // ESP_LOGI(TAGSTOMP, "Sending STOMP   sent len :%d  remain   %d\n", currentIndex,buffLen);
//             currentIndex+= CHANK_SIZE;
//             if(buffLen>0)buffLen= buffLen - CHANK_SIZE; // check bufflen 0 or not then calculate 
//             // vTaskDelay(30);
//         }

//     }while(buffLen!=0);


// return true;

// }




// bool imagesent(uint8_t* buff, uint16_t buffLen, uint8_t h, uint8_t w, char* name, uint16_t id, char* topic) {
//     // Calculate the required size for hex string
//     size_t tempLen = buffLen * 2 + 1; // 2 characters per byte + null terminator
//     char* hexString =  (char *)heap_caps_malloc(tempLen, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
//     if (hexString == NULL) {
//         ESP_LOGE(TAGSTOMP, "Memory allocation for hexString failed");
//         return false;
//     }

//     // Convert the entire buffer to a hex string
//     for (uint16_t i = 0; i < buffLen; i++) {
//         sprintf(&hexString[i * 2], "%02x", buff[i]);
//     }

//     // Null-terminate the hex string
//     hexString[tempLen - 1] = '\0';

//     uint16_t totalChank= (int)ceil((double)strlen(hexString) / IMAGE_CHANK_SIZE);


//     ESP_LOGW(TAGSTOMP, "Total hex string length: %d, Chunks to send: %d\n", strlen(hexString), totalChank);




//     // Send image info
//     char imageInfo[35];
//     snprintf(imageInfo, sizeof(imageInfo), "%d %d %d %s %d %d", buffLen, w, h, name, id,totalChank);
//     if (!stompSend(imageInfo, topic)) {
//         heap_caps_free(hexString);
//         return false;
//     }
//     vTaskDelay(50);

//     // Send the hex string in chunks
//     uint16_t currentIndex = 0;
//     uint16_t chankNo=0;
//     while (currentIndex < strlen(hexString)) {
//         char chunk[IMAGE_CHANK_SIZE + 1]; // Buffer for each chunk
//         memset(chunk, 0, sizeof(chunk));

//         // Calculate remaining length and copy chunk
//         size_t chunkLen = strlen(hexString) - currentIndex;
//         if (chunkLen > IMAGE_CHANK_SIZE) {
//             chunkLen = IMAGE_CHANK_SIZE;
//         }

//         strncpy(chunk, &hexString[currentIndex], chunkLen);

//         // Prepare the STOMP frame to send
//         char sentFrame[sizeof(chunk) + 55 + strlen(topic)];
//         memset(sentFrame, 0, sizeof(sentFrame));

//         snprintf(sentFrame, sizeof(sentFrame), "[\"SEND\\ndestination:%s\\n\\n%d %d %s\\n\\n\\u0000\"]", topic, id ,chankNo+1,chunk);

//         // printf("Chunk No: %d\n", chankNo+1);// chank no 


//         if (networkStatus != STOMP_CONNECTED) {
//             ESP_LOGE(TAGSTOMP, "Stomp disconnected\n");
//             vTaskDelay(50);
//             if(networkStatus>WIFI_CONNECTED)networkStatus = WSS_CONNECTED;   
//             vTaskDelay(100);
//             // free(hexString);
//             continue; // Retry sending
//         }
//         if (esp_websocket_client_send_text(client, sentFrame, strlen(sentFrame), portMAX_DELAY) == 0) {
//             vTaskDelay(50);
//             ESP_LOGI(TAGSTOMP, "STOMP send failed. Retrying...\n");
//             if(networkStatus>WIFI_CONNECTED)networkStatus = WSS_CONNECTED;   
//             vTaskDelay(100);



//             continue; // Retry sending

//         }else{
//             vTaskDelay(2);
//             currentIndex += chunkLen;
//             chankNo++;// no of chank 
//             float percentage_float = (chankNo /(float)totalChank) * 100;
//             percentage = (int)percentage_float;
            
//             // printf("total chank  %d send %d percentage: %d\n",totalChank,  chankNo,percentage);
//             if(percentage>=100)percentage=0;
//         }

//     }
//     heap_caps_free(hexString);
//     return true;
// }

//--------------------backup end



static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WSS_CONNECTED");
        networkStatus=WSS_CONNECTED;

        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WSS_DISCONNECTED");

        networkStatus=WIFI_CONNECTED;


        break;
    case WEBSOCKET_EVENT_DATA:

        if (data->op_code == 0x08 && data->data_len == 2) {

            ESP_LOGW(TAG, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);
                            // ESP_LOGI(TAG, "WEBSOCKET_free");
        // memset(data->data_ptr,0,data->data_len);

        } else {


            if (data->op_code == 0x0A) {

                ESP_LOGI(TAG, "Ping code: %d", data->op_code);


                time_library_time_t current_time;
                uint8_t clockType = get_time(&current_time, 1);

                uint8_t time [11];
                time[0]=(uint8_t)'T';
                time[1]=current_time.year-2000;
                time[2]=current_time.month;
                time[3]=current_time.day;
                
                time[4]=current_time.hour;
                time[5]=current_time.minute;
                time[6]=current_time.second;

                time[7]= day_names[calculate_day_of_week( current_time.year, current_time.month, current_time.day )][0];
                time[8]= day_names[calculate_day_of_week( current_time.year, current_time.month, current_time.day )][1];
                time[9]= day_names[calculate_day_of_week( current_time.year, current_time.month, current_time.day )][2];
                time[10]='\0';

                printf(" data len: %d time: %c %d %d %d %d %d %d %s\n",sizeof(time),time[0],time[1],time[2],time[3],time[4],time[5],time[6] ,(char*)&time[7]);
                stompS(time, sizeof(time));

            }

            
            if((char)data->data_ptr[0]=='T'){

                printf("rcv Time:  %d %d %d %d %d %d %c%c%c",data->data_ptr[1],data->data_ptr[2],data->data_ptr[3],data->data_ptr[4],data->data_ptr[5],data->data_ptr[6] ,data->data_ptr[7] ,data->data_ptr[8],data->data_ptr[9]);
                memset(data->data_ptr,0,data->data_len);

            }else if( (char)data->data_ptr[0]=='L' ) {

                ESP_LOGW(TAG, "Received log %.*s", data->data_len, (char *)data->data_ptr);
                memset(data->data_ptr,0,data->data_len);

            }else{


                ESP_LOGW(TAG, "Received:  %.*s", data->data_len, (char *)data->data_ptr);
                memset(data->data_ptr,0,data->data_len);


            }
            

        }

        // ESP_LOGI(TAG, "WEBSOCKET_free");





        // ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);



        // if (data->op_code == 0x08 && data->data_len == 2) {
        //     ESP_LOGE(TAG, "WSS Closed Code: %d", 256*data->data_ptr[0] + data->data_ptr[1]);

        //     vTaskDelay(100);
        //     break;

        // } else {

        //     if(data->data_ptr[0]=='o')stomp_client_connect();   

        //     else if(data->data_ptr[0]=='a'){

        //         ESP_LOGW(TAG, "Received= %s",(char *)data->data_ptr);
        //         stomp_client_handle_message(&data->data_ptr[3]);

        //     }

        //     else if(data->data_ptr[0]=='h'){
        //         //---------------------------------------------------------------
        //         if( networkStatus==STOMP_CONNECTED){
        //             ESP_LOGI(TAG, "Ping");
        //             time_library_time_t current_time;
        //             uint8_t clockType = get_time(&current_time, 1);
        //             char tempFrame[27];
        //             snprintf(tempFrame, sizeof(tempFrame), "%d %d %d %d %d %d %s",current_time.year,current_time.month,current_time.day,current_time.hour,current_time.minute,current_time.second,
        //             day_names[calculate_day_of_week( current_time.year, current_time.month, current_time.day )]);

        //             // if(!stompSend(tempFrame, PUBLISH_TOPIC)){
        //             //     // ESP_LOGI(TAGSTOMP, "sending error");

        //             // }

        //             uint8_t time [11];
        //             time[0]=(uint8_t)'T';
        //             time[1]=current_time.year-2000;
        //             time[2]=current_time.month;
        //             time[3]=current_time.day;
                    
        //             time[4]=current_time.hour;
        //             time[5]=current_time.minute;
        //             time[6]=current_time.second;

        //             time[7]= day_names[calculate_day_of_week( current_time.year, current_time.month, current_time.day )][0];
        //             time[8]= day_names[calculate_day_of_week( current_time.year, current_time.month, current_time.day )][1];
        //             time[9]= day_names[calculate_day_of_week( current_time.year, current_time.month, current_time.day )][2];
        //             time[10]='\0';

        //             printf(" data len: %d time: %c %d %d %d %d %d %d %s\n",sizeof(time),time[0],time[1],time[2],time[3],time[4],time[5],time[6] ,(char*)&time[7]);
        //             // stompS(time, sizeof(time), PUBLISH_TOPIC);
        //             // stompSend("(char*)time", PUBLISH_TOPIC);
        //             // vTaskDelay(100);

        //             // stompSend(time[0], PUBLISH_TOPIC);
        //             // stompSend((char*)time, PUBLISH_TOPIC);
        //             // stompS(&time, sizeof(time), PUBLISH_TOPIC);

        //         }
        //         //--------------------------------------------------------------
        //     }else if(data->data_ptr[0]=='c'){

        //         // ESP_LOGW(TAG, "Received= %s",(char *)data->data_ptr);
        //         networkStatus=WIFI_CONNECTED;
        //         stompAppStart();
        //     }
        //     // ESP_LOGI(TAG, "WEBSOCKET_free");
        //     memset(data->data_ptr,0,data->data_len);
        // }
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


    // int random1 = esp_random() % 999; // Generates a random number between 0 and 999
    // int random2 = esp_random() % 999999; // Generates a random number between 0 and 999999
    // snprintf(socket + strlen(socket), sizeof(socket) - strlen(socket), "%d/%d/websocket", random1, random2);


    websocket_cfg.uri = (const char*)socket;
    websocket_cfg.cert_pem = echo_org_ssl_ca_cert;  
    websocket_cfg.pingpong_timeout_sec=10;


    // char uinqeheaders[14];
    // snprintf(uinqeheaders, sizeof(uinqeheaders), "%s%08llu",DEVICE_VERSION_ID, generate_unique_id());
    // printf("\nheadr:  %s",uinqeheaders);

    // websocket_cfg.headers = (const char*)uinqeheaders;

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
    websocket_cfg.task_stack = 1024*4;  // Increased stack size
    websocket_cfg.task_prio =10;      // Set an appropriate task priority


    // ESP_LOGI(TAG, "Constructed WebSocket URL: %s", websocket_cfg.uri);
    // ESP_LOGI(TAG, "Constructed WebSocket PATH: %s", websocket_cfg.path);


    // ESP_LOGI(TAG, "Initializing global CA store...");
    ESP_ERROR_CHECK(esp_tls_set_global_ca_store((const unsigned char *)echo_org_ssl_ca_cert, sizeof(echo_org_ssl_ca_cert)));


    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);
}
