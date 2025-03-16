#include "WssClient.h"
#include "timeLib.h"
#include <math.h>
#include "esp_wifi.h"

#include "esp_blufi.h"

#include "esp_log.h"
#include "esp_tls.h"
#include "esp_system.h"


#define TAG "SSL_FETCHER"
#define SERVER_HOST "grozziieget.zjweiting.com"
#define SERVER_PORT 3091  // Your WebSocket server port


#define     TAG             "WSS"
#define     TAG_WSS        "WSS_CLIENT"

int8_t percentage=0;
int8_t maxTry=0;
uint8_t DataUpDoun=0;


bool sendToWss(uint8_t *buff, size_t buffLen) {
    uint8_t tempFrame[CHANK_SIZE];  // Buffer for each chunk
    uint16_t currentIndex = 0;

    // ESP_LOGW(TAG_WSS, "Total stompS length: %d", buffLen);
    DataUpDoun |= 1<<0;
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
            // ESP_LOGE(TAG_WSS, "Failed to send frame. maxTry...%d" ,maxTry);
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
            // ESP_LOGE(TAG_WSS, "Failed to send frame. Retrying...");
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

    ESP_LOGI(TAG_WSS , "📡 Sending Total Chunks : %d h %d w %d len:  %d\n\n",totalChunks, h  , w ,buffLen);
    

    // for(uint16_t i=0; i< buffLen;i++){

    //     printf("%02x",buff[i]);

    // }

    // printf("\n\n");

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

    // printf("Chunk done: %d \n",chunkNo);  // Log the chunk number
    return true;
}



static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGW(TAG, "WSS_CONNECTED");
        networkStatus=WSS_CONNECTED;
        send_custom_data_to_app("WSSCS"); // Send feedback to application over BLE

        if(!ble_is_connected){

            config=QR_CODE_SKIP;
            esp_blufi_adv_stop();
            blufi_security_init();

        }



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
        // ESP_LOGI(TAG, "WSS_DISCONNECTED");

        networkStatus=WIFI_CONNECTED;

        blufi_security_deinit();
        blufiAddStart();
        
        break;
    case WEBSOCKET_EVENT_DATA:

        if (data->op_code == 0x08 && data->data_len == 2) {

            // ESP_LOGW(TAG, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);
            networkStatus=WIFI_CONNECTED;
            memset(data->data_ptr,0,data->data_len);
            wssReset();

        } else {

            if (data->op_code == 0x0A) {

                // ESP_LOGI(TAG, "Ping code: %d", data->op_code);
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

                ESP_LOGI(TAG_WSS," D len: %d  Y%d M %d D %d W %d H %d MIN %d SEC %d FORMET %dH RSSI %d BATTRY %d \n",
                sizeof(time),time[1],time[2],time[3],time[4],time[5],time[6] ,time[7] ,time[8],time[9],batVoltage);

                sendToWss(time, sizeof(time));

            }else{

                ESP_LOGI(TAG, "Received: ");
                DataUpDoun |= 1<<1;

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
        // ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}

void wssReset(void){



    if (client != NULL) {
    esp_websocket_client_stop(client);
    esp_websocket_client_destroy(client);
    // ESP_LOGI(TAG, "❌ esp_websocket_client_stop");
    client = NULL;
    
    }      
    ESP_LOGI(TAG, "wssReset");
    vTaskDelay(500 / portTICK_PERIOD_MS);
    wssClientInt();
    vTaskDelay(1500 / portTICK_PERIOD_MS);

}

// old setup
void wssClientInt(void) {
   
    esp_websocket_client_config_t websocket_cfg = {};

    websocket_cfg.uri = (const char*)THT;
    websocket_cfg.cert_pem = echo_org_ssl_ca_cert; 
    websocket_cfg.ping_interval_sec= 5; 
    websocket_cfg.pingpong_timeout_sec=10;
    websocket_cfg.use_global_ca_store = true;
    websocket_cfg.skip_cert_common_name_check = true;
    websocket_cfg.disable_auto_reconnect = false;
    websocket_cfg.task_stack = 1024*5;//4
    websocket_cfg.task_prio =10; 
    // ESP_LOGI(TAG, "Initializing global CA store...");
    ESP_ERROR_CHECK(esp_tls_set_global_ca_store((const unsigned char *)echo_org_ssl_ca_cert, sizeof(echo_org_ssl_ca_cert)));

    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);

}

/*
// 🟢 Perform SSL Handshake with mbedTLS**
static int perform_mbedtls_handshake() {
   
    int ret;
    wss_client_t wss_client;
    // ESP_LOGI(TAG, "Initializing WebSocket Secure Client...");

    mbedtls_ssl_init(&wss_client.ssl);
    mbedtls_ctr_drbg_init(&wss_client.ctr_drbg);
    mbedtls_ssl_config_init(&wss_client.conf);
    mbedtls_entropy_init(&wss_client.entropy);

    // ESP_LOGI(TAG, "Seeding the random number generator...");
    if ((ret = mbedtls_ctr_drbg_seed(&wss_client.ctr_drbg, mbedtls_entropy_func, &wss_client.entropy, NULL, 0)) != 0) {
        // ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed failed: %d", ret);
        return;
    }

    // ESP_LOGI(TAG, "Setting up SSL/TLS structure...");
    if ((ret = mbedtls_ssl_config_defaults(&wss_client.conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        // ESP_LOGE(TAG, "mbedtls_ssl_config_defaults failed: %d", ret);
        return;
    }

    // Attach certificate bundle
    ret = esp_crt_bundle_attach(&wss_client.conf);
    if (ret < 0) {
        // ESP_LOGE(TAG, "esp_crt_bundle_attach failed: -0x%x", -ret);
        return;
    }

    // Set hostname for TLS verification
    if ((ret = mbedtls_ssl_set_hostname(&wss_client.ssl, WEB_SERVER)) != 0) {
        // ESP_LOGE(TAG, "mbedtls_ssl_set_hostname failed: -0x%x", -ret);
        return;
    }

    mbedtls_ssl_conf_authmode(&wss_client.conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_rng(&wss_client.conf, mbedtls_ctr_drbg_random, &wss_client.ctr_drbg);

    if ((ret = mbedtls_ssl_setup(&wss_client.ssl, &wss_client.conf)) != 0) {
        // ESP_LOGE(TAG, "mbedtls_ssl_setup failed: -0x%x", -ret);
        return;
    }


    // Connect to WebSocket Server
    // 🟢 **Ensure old TCP socket is closed before reconnecting**
    mbedtls_net_free(&wss_client.server_fd);  // Close previous TCP socket
    mbedtls_net_init(&wss_client.server_fd);  // Reinitialize it

    // ESP_LOGI(TAG, "Connecting to WebSocket server: %s:%s...", WEB_SERVER, WEB_PORT);
    if ((ret = mbedtls_net_connect(&wss_client.server_fd, WEB_SERVER, WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0) {
        // ESP_LOGE(TAG, "mbedtls_net_connect failed: -0x%x", -ret);
        return;
    }

    // ESP_LOGI(TAG, "Connected to WebSocket server.");

    mbedtls_ssl_set_bio(&wss_client.ssl, &wss_client.server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    // ESP_LOGI(TAG, "Performing SSL/TLS handshake...");
    while ((ret = mbedtls_ssl_handshake(&wss_client.ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            // ESP_LOGE(TAG, "mbedtls_ssl_handshake failed: -0x%x", -ret);
            return;
        }
    }



    // ESP_LOGI(TAG, "TLS handshake successful! Extracting SSL certificate...");

    //  **Step 2: Extract the SSL Certificate**

    // // 🟢 Extract and Convert SSL Certificate to PEM
    // const mbedtls_x509_crt *server_cert = mbedtls_ssl_get_peer_cert(&wss_client.ssl);
    // if (!server_cert) {
    //     ESP_LOGE(TAG, "Failed to retrieve SSL certificate!");
    //     return -1;
    // }

    // // Get the certificate length
    // size_t cert_len = server_cert->raw.len;
    // if (cert_len <= 0 || cert_len > 4096) {
    //     ESP_LOGE(TAG, "Invalid certificate length: %d", cert_len);
    //     return -1;
    // }

    // // Allocate memory for Base64 PEM certificate
    // size_t pem_cert_len = cert_len * 2;
    // ssl_cert_pem = (uint8_t *)heap_caps_malloc(pem_cert_len, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    // if (!ssl_cert_pem) {
    //     ESP_LOGE(TAG, "Failed to allocate memory for SSL certificate!");
    //     return -1;
    // }

    // // Convert DER to PEM (Base64 encoding)
    // size_t olen = 0;
    // ret = mbedtls_base64_encode(ssl_cert_pem, pem_cert_len, &olen, server_cert->raw.p, cert_len);
    // if (ret != 0) {
    //     ESP_LOGE(TAG, "Failed to convert certificate to PEM format! Error: -0x%x", -ret);
    //     heap_caps_free(ssl_cert_pem);
    //     return -1;
    // }

    // // Add PEM headers
    // char *final_cert = (char *)heap_caps_malloc(olen + 64, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    // if (!final_cert) {
    //     ESP_LOGE(TAG, "Failed to allocate memory for PEM certificate!");
    //     heap_caps_free(ssl_cert_pem);
    //     return -1;
    // }

    // sprintf(final_cert, "-----BEGIN CERTIFICATE-----\n%s\n-----END CERTIFICATE-----\n", ssl_cert_pem);
    // heap_caps_free(ssl_cert_pem);
    // ssl_cert_pem = (uint8_t *)final_cert;  // Assign the properly formatted PEM certificate

    // ESP_LOGI(TAG, "Extracted SSL Certificate (PEM Format):\n%s", ssl_cert_pem);


    // 🟢  Get the first certificate (server certificate)
    const mbedtls_x509_crt *server_cert = mbedtls_ssl_get_peer_cert(&wss_client.ssl);
    if (!server_cert) {
        // ESP_LOGE(TAG, "Failed to retrieve SSL certificate!");
        return -1;
    }

    // 🟢  Get the Root CA (last certificate in the chain)
    const mbedtls_x509_crt *root_cert = server_cert;
    while (root_cert->next != NULL) {  // Traverse the certificate chain
        root_cert = root_cert->next;
    }

    // ESP_LOGI(TAG, "Extracting Root CA Certificate...");

    // 🟢  Get the certificate length
    size_t cert_len = root_cert->raw.len;
    if (cert_len <= 0 || cert_len > 4096) {
        // ESP_LOGE(TAG, "Invalid Root CA certificate length: %d", cert_len);
        return -1;
    }

    // 🟢  Allocate memory dynamically in SPI RAM
    size_t pem_cert_len = cert_len * 2;  // Base64 encoding increases size
    ssl_cert_pem = (uint8_t *)heap_caps_malloc(pem_cert_len, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (!ssl_cert_pem) {
        // ESP_LOGE(TAG, "Failed to allocate memory for Root CA certificate!");
        return -1;
    }

    // 🟢  Convert DER to PEM format
    size_t olen = 0;
    ret = mbedtls_base64_encode(ssl_cert_pem, pem_cert_len, &olen, root_cert->raw.p, cert_len);
    if (ret != 0) {
        // ESP_LOGE(TAG, "Failed to convert Root CA to PEM format! Error: -0x%x", -ret);
        heap_caps_free(ssl_cert_pem);
        return -1;
    }

    // 🟢  Add PEM headers
    char *final_cert = (char *)heap_caps_malloc(olen + 64, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (!final_cert) {
        // ESP_LOGE(TAG, "Failed to allocate memory for Root CA PEM certificate!");
        heap_caps_free(ssl_cert_pem);
        return -1;
    }

    sprintf(final_cert, "-----BEGIN CERTIFICATE-----\n%s\n-----END CERTIFICATE-----\n", ssl_cert_pem);
    heap_caps_free(ssl_cert_pem);
    ssl_cert_pem = (uint8_t *)final_cert;  // Assign the properly formatted PEM certificate


    // ESP_LOGI(TAG, "✅ Extracted Root CA Certificate (PEM Format):\n%s", ssl_cert_pem);

    // Cleanup
    mbedtls_ssl_free(&wss_client.ssl);
    mbedtls_ssl_config_free(&wss_client.conf);
    mbedtls_ctr_drbg_free(&wss_client.ctr_drbg);
    mbedtls_entropy_free(&wss_client.entropy);
    mbedtls_net_free(&wss_client.server_fd);
    // heap_caps_free(final_cert);


    return 0;
}
*/
/*
void wssClientInt(void) {
    ESP_LOGI(TAG, "🔗 Initializing WebSocket...");


    if (ssl_cert_pem == NULL) {
        // First, perform SSL handshake using mbedTLS
        if (perform_mbedtls_handshake() != 0) {
            // ESP_LOGE(TAG, "❌ Failed to perform SSL handshake!");
            return;
        }

        // Ensure SSL certificate is valid
        if (ssl_cert_pem == NULL) {
            // ESP_LOGE(TAG, "❌ SSL Certificate is NULL! Cannot proceed with WebSocket connection.");
            return;
        }

        // 🟢 Set Global CA Store (Only Once)
        size_t cert_len = strlen((const char *)ssl_cert_pem) + 1;  // Get actual length
        // ESP_LOGI(TAG, "🔐 Setting global CA store...");

        esp_err_t ret = esp_tls_set_global_ca_store((const unsigned char *)ssl_cert_pem, cert_len);
        if (ret != ESP_OK) {
            // ESP_LOGE(TAG, "❌ Failed to set global CA store! Error: %d", ret);
            return;
        }
    }

    // 🟢 Configure WebSocket Client
    esp_websocket_client_config_t websocket_cfg = {
        .uri = (const char*)THT,
        .cert_pem = (const char *)ssl_cert_pem,
        .ping_interval_sec = 5,
        .pingpong_timeout_sec = 10,
        .use_global_ca_store = true,  // Now we correctly set the CA store
        .skip_cert_common_name_check = true,
        .disable_auto_reconnect = false,
        .task_stack = 1024 * 5,  // Increase if needed
        .task_prio = 15,
    };

    // 🟢 Initialize WebSocket
    ESP_LOGI(TAG, "📡 Connecting to WebSocket server...");
    client = esp_websocket_client_init(&websocket_cfg);
    if (client == NULL) {
        // ESP_LOGE(TAG, "❌ Failed to initialize WebSocket client!");
        return;
    }

    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);

}
    */

