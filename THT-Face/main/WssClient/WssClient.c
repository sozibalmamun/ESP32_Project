#include "WssClient.h"
#include "timeLib.h"
#include <math.h>
#include "esp_wifi.h"
#define     TAG             "WSS"
#define     TAG_WSS        "WSS_CLIENT"

int8_t percentage=0;
int8_t maxTry=0;



bool sendToWss(uint8_t *buff, size_t buffLen) {
    uint8_t tempFrame[CHANK_SIZE];  // Buffer for each chunk
    uint16_t currentIndex = 0;

    // ESP_LOGW(TAG_WSS, "Total stompS length: %d", buffLen);

    while (buffLen > 0) {
        // Prepare the chunk data
        memset(tempFrame, 0, sizeof(tempFrame));
        size_t chunkLen = MIN(CHANK_SIZE, buffLen);
        memcpy(tempFrame, &buff[currentIndex], chunkLen);

        // Calculate the required size for the sending frame (adjust for extra headers)
        size_t sendingFrameLen = chunkLen + 80;  // Adjust based on format
        char* sendingFrame = (char*)heap_caps_malloc(sendingFrameLen + 1, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        if (sendingFrame == NULL) {
            return false;  // Memory allocation failure
        }
        memset(sendingFrame, 0, sendingFrameLen + 1);

        // Generate unique ID
        char uniqueID[10];
        snprintf(uniqueID, sizeof(uniqueID), "%08llu", generate_unique_id());

        // Prepare the STOMP frame
        strcat(sendingFrame, "DEVICE_TYPE:UFACE\\n");
        strcat(sendingFrame, "ID:");
        strcat(sendingFrame, DEVICE_VERSION_ID);
        strcat(sendingFrame, uniqueID);
        strcat(sendingFrame, "\\nDATA_TYPE:B\\nDATA:");

        // Add the binary chunk data to the sending frame using memcpy
        size_t headerLen = strlen(sendingFrame);
        memcpy(&sendingFrame[headerLen], tempFrame, chunkLen);

        // Append frame terminator
        strcat(sendingFrame + headerLen + chunkLen, "\\n\\n\\u0000");

        // ESP_LOGW(TAG_WSS, "wss pac: %s", sendingFrame);

// debug protocall bin data
        // for(uint16_t i=0; i< sendingFrameLen;i++){

        //     printf("%02x ",sendingFrame[i]);

        // }

        if (networkStatus != WSS_CONNECTED) {
            if (networkStatus > WIFI_DISS) networkStatus = WIFI_CONNECTED;
            heap_caps_free(sendingFrame);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            ESP_LOGE(TAG_WSS, "Failed to send frame. maxTry...%d" ,maxTry);
            maxTry++;
            if (maxTry > MAXTRY) {
                maxTry = 0;
                return false;
            }else if(maxTry == 5){
                wssReset();
            }
            continue;
        }

        // Send the prepared frame via WebSocket (replace with your WebSocket send function)
        if (esp_websocket_client_send_bin(client, sendingFrame, headerLen + chunkLen + 6, portMAX_DELAY) == 0) {
            ESP_LOGE(TAG_WSS, "Failed to send frame. Retrying...");
            heap_caps_free(sendingFrame);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;  // Retry if failed to send
        }

        // Update index and remaining length
        currentIndex += chunkLen;
        buffLen -= chunkLen;
        maxTry = 0;


        // Free allocated memory
        heap_caps_free(sendingFrame);
    }

    return true;
}

bool imagesent(uint8_t* buff, uint16_t buffLen, uint8_t h, uint8_t w, char* name, uint16_t id) {

    uint16_t totalChunks = (buffLen + IMAGE_CHANK_SIZE - 1) / IMAGE_CHANK_SIZE;  // Total number of chunks

    printf("totalChunks : %d h %d w %d image len:  %d\n\n",totalChunks, h  , w ,buffLen);


    // for(uint16_t i=0; i< buffLen;i++){

    //     printf("%02x",buff[i]);

    // }

    printf("\n\n");

//--------------------------- start of info sent--------------------------------------------------------------

    // Prepare image info
    char imageInfo[35] = {0};
    imageInfo[0] = (buffLen >> 8) & 0xFF;               // High byte of buffLen
    imageInfo[1] = buffLen & 0xFF;                      // Low byte of buffLen
    imageInfo[2] = ' ';                                 // space

    imageInfo[3] = h;                                   // Image height
    imageInfo[4] = ' ';                                 // space

    imageInfo[5] = w;                                   // Image width
    imageInfo[6] = ' ';                                 // space

    memcpy(&imageInfo[7], name, strlen(name));          // Copy the name
    imageInfo[6 + strlen(name)] = ' ';                  // space

    imageInfo[7 + strlen(name)] = (id >> 8) & 0xFF;     // High byte of ID
    imageInfo[8 + strlen(name)] = id & 0xFF;            // Low byte of ID
    imageInfo[9] = ' ';                                 // space

    imageInfo[10 + strlen(name)] = totalChunks & 0xFF;  // Total chunks

    // Send image info
    size_t imageInfoLen = 7 + strlen(name);             // Calculate the length of imageInfo
    if (!sendToWss((uint8_t*)imageInfo, imageInfoLen)) {
        // printf("Image info send failed\n");
        return false;
    }
//--------------------------- end of info sent--------------------------------------------------------------
    vTaskDelay(50);
   
    uint16_t currentIndex = 0;
    uint16_t chunkNo = 0;

    // Send the image chunks
    while (currentIndex < buffLen) {
        size_t chunkLen = (buffLen - currentIndex) > IMAGE_CHANK_SIZE ? IMAGE_CHANK_SIZE : (buffLen - currentIndex);
        uint8_t* chunk = (uint8_t*)heap_caps_malloc(chunkLen, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        if (chunk == NULL) {
            return false;  // Handle memory allocation failure
        }
        memcpy(chunk, &buff[currentIndex], chunkLen);  // Copy chunk of image data

        // Prepare the frame to send: id + chunkNo + chunk data
        size_t sentFrameLen = chunkLen + 9;  // 2 bytes for ID, 1 byte for chunkNo+3 space+ 2byte for crc
        uint8_t* sentFrame = (uint8_t*)heap_caps_malloc(sentFrameLen, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        if (sentFrame == NULL) {
            heap_caps_free(chunk);  // Free the allocated chunk before returning

            return false;
        }
        const uint16_t Crc = crc16((char*)chunk,chunkLen);
        

        // Pack the frame with ID, chunk number, and chunk data
        sentFrame[0] = (id >> 8) & 0xFF;     // High byte of ID
        sentFrame[1] = id & 0xFF;            // Low byte of ID
        sentFrame[2] = ' ';                  // space
        sentFrame[3] = chunkNo + 1;          // Current chunk number
        sentFrame[4] = ' ';                  // space

        sentFrame[5] = Crc>>8;               //crc high
        sentFrame[6] = Crc&0xFF;             //crc low

        sentFrame[7] = ' ';                  // space


        memcpy(&sentFrame[8], chunk, chunkLen);  // Copy chunk data to the frame

        // printf("Chunk No: %d CRC: %x\n", chunkNo + 1 ,Crc);  // Log the chunk number

        // Send the frame
        if (!sendToWss(sentFrame, sentFrameLen)) {
            heap_caps_free(chunk);
            heap_caps_free(sentFrame);
            percentage=0;
            return false;
        }

        // Update the current index and chunk number
        currentIndex += chunkLen;
        chunkNo++;

        float percentage_float = (chunkNo /(float)totalChunks) * 100;
        percentage = (int)percentage_float;

        // printf("total chank  %d send %d percentage: %d\n",totalChank,  chankNo,percentage);
        if(percentage>=100)percentage=0;
        // Free allocated memory after sending
        heap_caps_free(chunk);
        heap_caps_free(sentFrame);
    }

    printf("Chunk done: %d \n",chunkNo);  // Log the chunk number
    return true;
}



static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WSS_CONNECTED");
        networkStatus=WSS_CONNECTED;


        time_library_time_t current_time;
        get_time(&current_time, dspTimeFormet);
        wifi_ap_record_t ap_info;
        esp_wifi_sta_get_ap_info(&ap_info);
        int32_t rssi = ap_info.rssi;
        uint8_t wifiRssi = wifi_rssi_to_percentage(rssi);
        // ESP_LOGI("WiFi", "Current Wi-Fi RSSI: %d dBm (%d%%)", rssi, wifiRssi);

        uint8_t time [13];
        time[0]=(uint8_t)'T';
        time[1]=current_time.year-2000;
        time[2]=current_time.month;
        time[3]=current_time.day;
        time[4]= current_time.weekday;

        time[5]=current_time.hour;
        time[6]=current_time.minute;
        time[7]=current_time.second;
        time[8]= dspTimeFormet==true?0x0C:0x18;// sent time formet
        time[9]= wifiRssi;
        time[10]=batVoltage << 8;
        time[11]=batVoltage & 0xFF;
        time[12]='\0';

        // printf(" D len: %d  Y%d M %d D %d W %d H %d MIN %d SEC %d FORMET %dH RSSI %d BATTRY %d \n",
        // sizeof(time),time[1],time[2],time[3],time[4],time[5],time[6] ,time[7] ,time[8],time[9],batVoltage);

        sendToWss(time, sizeof(time));



        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WSS_DISCONNECTED");

        networkStatus=WIFI_CONNECTED;


        break;
    case WEBSOCKET_EVENT_DATA:

        if (data->op_code == 0x08 && data->data_len == 2) {

            ESP_LOGW(TAG, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);
            networkStatus=WIFI_CONNECTED;
            memset(data->data_ptr,0,data->data_len);
            wssReset();

        } else {

            if (data->op_code == 0x0A) {

                ESP_LOGI(TAG, "Ping code: %d", data->op_code);
                time_library_time_t current_time;
                get_time(&current_time, dspTimeFormet);
                wifi_ap_record_t ap_info;
                esp_wifi_sta_get_ap_info(&ap_info);
                int32_t rssi = ap_info.rssi;
                uint8_t wifiRssi = wifi_rssi_to_percentage(rssi);
                // ESP_LOGI("WiFi", "Current Wi-Fi RSSI: %d dBm (%d%%)", rssi, wifiRssi);
    

                uint8_t time [13];
                time[0]=(uint8_t)'T';
                time[1]=current_time.year-2000;
                time[2]=current_time.month;
                time[3]=current_time.day;
                time[4]= current_time.weekday;

                time[5]=current_time.hour;
                time[6]=current_time.minute;
                time[7]=current_time.second;
                time[8]= dspTimeFormet==true?0x0C:0x18;// sent time formet
                time[9]= wifiRssi;
                time[10]=batVoltage << 8;
                time[11]=batVoltage & 0xFF;
                time[12]='\0';

                printf(" D len: %d  Y%d M %d D %d W %d H %d MIN %d SEC %d FORMET %dH RSSI %d BATTRY %d \n",
                sizeof(time),time[1],time[2],time[3],time[4],time[5],time[6] ,time[7] ,time[8],time[9],batVoltage);

                sendToWss(time, sizeof(time));

            }else{

                // ESP_LOGE(TAG, "Received: ");
                vTaskDelay(50);

                // for(uint16_t i=0; i< data->data_len;i++){

                //     printf("%x ",data->data_ptr[i]);

                // }
                process_command((char *)data->data_ptr);
                memset(data->data_ptr,0,data->data_len);

            }



        }

        // ESP_LOGI(TAG, "WEBSOCKET_free");

        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}


void wssClientInt(void) {
   
    esp_websocket_client_config_t websocket_cfg = {};


    websocket_cfg.uri = (const char*)THT;
    websocket_cfg.cert_pem = echo_org_ssl_ca_cert; 
    websocket_cfg.ping_interval_sec= 5; 
    websocket_cfg.pingpong_timeout_sec=10;
    websocket_cfg.use_global_ca_store = true;
    websocket_cfg.skip_cert_common_name_check = true;
    websocket_cfg.disable_auto_reconnect = false;
    websocket_cfg.task_stack = 1024*4;  
    websocket_cfg.task_prio =10; 

    // ESP_LOGI(TAG, "Initializing global CA store...");
    ESP_ERROR_CHECK(esp_tls_set_global_ca_store((const unsigned char *)echo_org_ssl_ca_cert, sizeof(echo_org_ssl_ca_cert)));


    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);
}

void wssReset(void){



    if (client != NULL) {
    esp_websocket_client_stop(client);
    esp_websocket_client_destroy(client);
    ESP_LOGI(TAG, "esp_websocket_client_stop");
    client = NULL;
    
    }            
    vTaskDelay(50);
    wssClientInt();
    ESP_LOGI(TAG, "wssAppStart");

}





// void wssAppStart(void)
// {

//     stompInfo_cfg_t stompInfo ={
//         .uri = THT,
//         .host = HOST,
//         .port = PORT,
//         .path = PATH
//     };
//     wss_client_int(stompInfo);
// }

// void wss_client_int( stompInfo_cfg_t stompSetup ) {
   
//     esp_websocket_client_config_t websocket_cfg = {};

//     // char socket[100];
//     // snprintf(socket, sizeof(socket), "%s", stompSetup.uri);


//     // int random1 = esp_random() % 999; // Generates a random number between 0 and 999
//     // int random2 = esp_random() % 999999; // Generates a random number between 0 and 999999
//     // snprintf(socket + strlen(socket), sizeof(socket) - strlen(socket), "%d/%d/websocket", random1, random2);


//     websocket_cfg.uri = (const char*)THT;
//     websocket_cfg.cert_pem = echo_org_ssl_ca_cert;  
//     websocket_cfg.pingpong_timeout_sec=10;


//     // char uinqeheaders[14];
//     // snprintf(uinqeheaders, sizeof(uinqeheaders), "%s%08llu",DEVICE_VERSION_ID, generate_unique_id());
//     // printf("\nheadr:  %s",uinqeheaders);

//     // websocket_cfg.headers = (const char*)uinqeheaders;

//     // memset(socket,0,strlen(socket));

//     // snprintf(socket, sizeof(socket), "%s", stompSetup.path);
//     // snprintf(socket + strlen(socket), sizeof(socket) - strlen(socket), "%d/%d/websocket", random1, random2);
    
//     // websocket_cfg.path = (const char*)socket;// path
//     // websocket_cfg.port = stompSetup.port;
//     // websocket_cfg.host = (const char*)stompSetup.host;

//     // websocket_cfg.use_global_ca_store = false;// try 
//     // websocket_cfg.skip_cert_common_name_check = false;
//     // websocket_cfg.disable_auto_reconnect = false;

//     websocket_cfg.use_global_ca_store = true;// ok 
//     websocket_cfg.skip_cert_common_name_check = true;
//     websocket_cfg.disable_auto_reconnect = false;
//     websocket_cfg.task_stack = 1024*4;  // Increased stack size
//     websocket_cfg.task_prio =10;      // Set an appropriate task priority


//     // ESP_LOGI(TAG, "Constructed WebSocket URL: %s", websocket_cfg.uri);
//     // ESP_LOGI(TAG, "Constructed WebSocket PATH: %s", websocket_cfg.path);


//     // ESP_LOGI(TAG, "Initializing global CA store...");
//     ESP_ERROR_CHECK(esp_tls_set_global_ca_store((const unsigned char *)echo_org_ssl_ca_cert, sizeof(echo_org_ssl_ca_cert)));


//     client = esp_websocket_client_init(&websocket_cfg);
//     esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
//     esp_websocket_client_start(client);
// }


