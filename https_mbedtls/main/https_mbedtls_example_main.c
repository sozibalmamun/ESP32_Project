// /* HTTPS GET Example using plain mbedTLS sockets
//  *
//  * Contacts the howsmyssl.com API via TLS v1.2 and reads a JSON
//  * response.
//  *
//  * Adapted from the ssl_client1 example in mbedtls.
//  *
//  * Original Copyright (C) 2006-2016, ARM Limited, All Rights Reserved, Apache 2.0 License.
//  * Additions Copyright (C) Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD, Apache 2.0 License.
//  *
//  *
//  * Licensed under the Apache License, Version 2.0 (the "License");
//  * you may not use this file except in compliance with the License.
//  * You may obtain a copy of the License at
//  *
//  *     http://www.apache.org/licenses/LICENSE-2.0
//  *
//  * Unless required by applicable law or agreed to in writing, software
//  * distributed under the License is distributed on an "AS IS" BASIS,
//  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  * See the License for the specific language governing permissions and
//  * limitations under the License.
//  */
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "esp_crt_bundle.h"


// /* Constants that aren't configurable in menuconfig */
// #define WEB_SERVER "grozziieget.zjweiting.com"
// #define WEB_PORT "3091"
// #define WEB_URL "https://grozziieget.zjweiting.com/"

// static const char *TAG = "example";

// static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
//     "Host: "WEB_SERVER"\r\n"
//     "User-Agent: esp-idf/1.0 esp32\r\n"
//     "\r\n";


// static void https_get_task(void *pvParameters)
// {
//     char buf[512];
//     int ret, flags, len;

//     mbedtls_entropy_context entropy;
//     mbedtls_ctr_drbg_context ctr_drbg;
//     mbedtls_ssl_context ssl;
//     mbedtls_x509_crt cacert;
//     mbedtls_ssl_config conf;
//     mbedtls_net_context server_fd;

//     mbedtls_ssl_init(&ssl);
//     mbedtls_x509_crt_init(&cacert);
//     mbedtls_ctr_drbg_init(&ctr_drbg);
//     ESP_LOGI(TAG, "Seeding the random number generator");

//     mbedtls_ssl_config_init(&conf);

//     mbedtls_entropy_init(&entropy);
//     if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
//                                     NULL, 0)) != 0)
//     {
//         ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
//         abort();
//     }

//     ESP_LOGI(TAG, "Attaching the certificate bundle...");

//     ret = esp_crt_bundle_attach(&conf);

//     if(ret < 0)
//     {
//         ESP_LOGE(TAG, "esp_crt_bundle_attach returned -0x%x\n\n", -ret);
//         abort();
//     }

//     ESP_LOGI(TAG, "Setting hostname for TLS session...");

//      /* Hostname set here should match CN in server certificate */
//     if((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
//     {
//         ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
//         abort();
//     }

//     ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");

//     if((ret = mbedtls_ssl_config_defaults(&conf,
//                                           MBEDTLS_SSL_IS_CLIENT,
//                                           MBEDTLS_SSL_TRANSPORT_STREAM,
//                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
//     {
//         ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
//         goto exit;
//     }

//     /* MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example it will print
//        a warning if CA verification fails but it will continue to connect.

//        You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your own code.
//     */
//     mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
//     mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
//     mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
// #ifdef CONFIG_MBEDTLS_DEBUG
//     mbedtls_esp_enable_debug_log(&conf, CONFIG_MBEDTLS_DEBUG_LEVEL);
// #endif

//     if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
//     {
//         ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
//         goto exit;
//     }

//     while(1) {
//         mbedtls_net_init(&server_fd);

//         ESP_LOGI(TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);

//         if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
//                                       WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
//         {
//             ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
//             goto exit;
//         }

//         ESP_LOGI(TAG, "Connected.");

//         mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

//         ESP_LOGI(TAG, "Performing the SSL/TLS handshake...");

//         while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
//         {
//             if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
//             {
//                 ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
//                 goto exit;
//             }
//         }

//         ESP_LOGI(TAG, "Verifying peer X.509 certificate...");

//         if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
//         {
//             /* In real life, we probably want to close connection if ret != 0 */
//             ESP_LOGW(TAG, "Failed to verify peer certificate!");
//             bzero(buf, sizeof(buf));
//             mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
//             ESP_LOGW(TAG, "verification info: %s", buf);
//         }
//         else {
//             ESP_LOGI(TAG, "Certificate verified.");
//         }

//         ESP_LOGI(TAG, "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));

//         ESP_LOGI(TAG, "Writing HTTP request...");

//         size_t written_bytes = 0;
//         do {
//             ret = mbedtls_ssl_write(&ssl,
//                                     (const unsigned char *)REQUEST + written_bytes,
//                                     strlen(REQUEST) - written_bytes);
//             if (ret >= 0) {
//                 ESP_LOGI(TAG, "%d bytes written", ret);
//                 written_bytes += ret;
//             } else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ) {
//                 ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
//                 goto exit;
//             }
//         } while(written_bytes < strlen(REQUEST));

//         ESP_LOGI(TAG, "Reading HTTP response...");

//         do
//         {
//             len = sizeof(buf) - 1;
//             bzero(buf, sizeof(buf));
//             ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, len);

//             if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
//                 continue;

//             if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
//                 ret = 0;
//                 break;
//             }

//             if(ret < 0)
//             {
//                 ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
//                 break;
//             }

//             if(ret == 0)
//             {
//                 ESP_LOGI(TAG, "connection closed");
//                 break;
//             }

//             len = ret;
//             ESP_LOGD(TAG, "%d bytes read", len);
//             /* Print response directly to stdout as it is read */
//             for(int i = 0; i < len; i++) {
//                 putchar(buf[i]);
//             }
//         } while(1);

//         mbedtls_ssl_close_notify(&ssl);

//     exit:
//         mbedtls_ssl_session_reset(&ssl);
//         mbedtls_net_free(&server_fd);

//         if(ret != 0)
//         {
//             mbedtls_strerror(ret, buf, 100);
//             ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, buf);
//         }

//         putchar('\n'); // JSON output doesn't have a newline at end

//         static int request_count;
//         ESP_LOGI(TAG, "Completed %d requests", ++request_count);
//         printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

//         for(int countdown = 10; countdown >= 0; countdown--) {
//             ESP_LOGI(TAG, "%d...", countdown);
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//         }
//         ESP_LOGI(TAG, "Starting again!");
//     }
// }

// void app_main(void)
// {
//     ESP_ERROR_CHECK( nvs_flash_init() );
//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());

//     /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
//      * Read "Establishing Wi-Fi or Ethernet Connection" section in
//      * examples/protocols/README.md for more information about this function.
//      */
//     ESP_ERROR_CHECK(example_connect());

//     xTaskCreate(&https_get_task, "https_get_task", 8192, NULL, 5, NULL);
// }



#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/sockets.h"

#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "esp_crt_bundle.h"

// #define TAG "WSS_CLIENT"
// #define WEB_SERVER "grozziieget.zjweiting.com"
// #define WEB_PORT "3091"
// #define WEB_PATH "/WebSocket-Binary/ws"

// // WebSocket Handshake Request
// static const char *WEBSOCKET_HANDSHAKE =
//     "GET " WEB_PATH " HTTP/1.1\r\n"
//     "Host: " WEB_SERVER ":" WEB_PORT "\r\n"
//     "Connection: Upgrade\r\n"
//     "Upgrade: websocket\r\n"
//     "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"  // Base64-encoded key
//     "Sec-WebSocket-Version: 13\r\n"
//     "\r\n";

// void websocket_client_task(void *pvParameters) {
//     char buf[1024];
//     int ret, len;
    
//     mbedtls_entropy_context entropy;
//     mbedtls_ctr_drbg_context ctr_drbg;
//     mbedtls_ssl_context ssl;
//     mbedtls_ssl_config conf;
//     mbedtls_net_context server_fd;

//     // Initialize TLS structures
//     mbedtls_ssl_init(&ssl);
//     mbedtls_ctr_drbg_init(&ctr_drbg);
//     mbedtls_ssl_config_init(&conf);
//     mbedtls_entropy_init(&entropy);

//     ESP_LOGI(TAG, "Seeding the random number generator...");
//     if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0)) != 0) {
//         ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
//         abort();
//     }

//     ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");
//     if ((ret = mbedtls_ssl_config_defaults(&conf,
//                                            MBEDTLS_SSL_IS_CLIENT,
//                                            MBEDTLS_SSL_TRANSPORT_STREAM,
//                                            MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
//         ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
//         goto exit;
//     }

//     // Attach certificate bundle
//     ret = esp_crt_bundle_attach(&conf);
//     if (ret < 0) {
//         ESP_LOGE(TAG, "esp_crt_bundle_attach failed -0x%x", -ret);
//         abort();
//     }

//     // Set hostname for TLS verification
//     if ((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0) {
//         ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
//         goto exit;
//     }

//     mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);  // Change to VERIFY_REQUIRED for stronger security
//     mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
    
//     if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) {
//         ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x", -ret);
//         goto exit;
//     }

//     while (1) {
//         mbedtls_net_init(&server_fd);

//         ESP_LOGI(TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);
//         if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER, WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0) {
//             ESP_LOGE(TAG, "mbedtls_net_connect returned -0x%x", -ret);
//             goto exit;
//         }

//         ESP_LOGI(TAG, "Connected to server.");

//         mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

//         ESP_LOGI(TAG, "Performing SSL/TLS handshake...");
//         while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
//             if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
//                 ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
//                 goto exit;
//             }
//         }

//         ESP_LOGI(TAG, "TLS handshake successful. Sending WebSocket handshake request...");

//         // Send WebSocket Upgrade Request
//         size_t written_bytes = 0;
//         do {
//             ret = mbedtls_ssl_write(&ssl, (const unsigned char *)WEBSOCKET_HANDSHAKE + written_bytes,
//                                     strlen(WEBSOCKET_HANDSHAKE) - written_bytes);
//             if (ret >= 0) {
//                 written_bytes += ret;
//             } else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ) {
//                 ESP_LOGE(TAG, "mbedtls_ssl_write failed -0x%x", -ret);
//                 goto exit;
//             }
//         } while (written_bytes < strlen(WEBSOCKET_HANDSHAKE));

//         ESP_LOGI(TAG, "Waiting for WebSocket handshake response...");

//         // Read WebSocket handshake response
//         do {
//             len = sizeof(buf) - 1;
//             bzero(buf, sizeof(buf));
//             ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, len);
//             if (ret < 0) {
//                 ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
//                 goto exit;
//             } else if (ret == 0) {
//                 ESP_LOGI(TAG, "Connection closed.");
//                 break;
//             }

//             buf[ret] = '\0';  // Null terminate response
//             ESP_LOGI(TAG, "Received WebSocket handshake response: \n%s", buf);
//         } while (1);

//         ESP_LOGI(TAG, "WebSocket connection established. Ready to send and receive data!");

//         // TODO: Send WebSocket frames using proper WebSocket format

//     exit:
//         mbedtls_ssl_session_reset(&ssl);
//         mbedtls_net_free(&server_fd);

//         if (ret != 0) {
//             char error_buf[100];
//             mbedtls_strerror(ret, error_buf, 100);
//             ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, error_buf);
//         }

//         vTaskDelay(10000 / portTICK_PERIOD_MS);  // Retry after 10 seconds
//     }
// }

// void app_main(void) {
//     ESP_ERROR_CHECK(nvs_flash_init());
//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());
//     ESP_ERROR_CHECK(example_connect());  // Connect to Wi-Fi

//     xTaskCreate(&websocket_client_task, "websocket_client_task", 8192, NULL, 5, NULL);
// }



#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"

#define TAG "WSS_CLIENT"

#define WEB_SERVER "grozziieget.zjweiting.com"
#define WEB_PORT "3091"
#define WEB_PATH "/WebSocket-Binary/ws"

// WebSocket Handshake Request
static const char *WEBSOCKET_HANDSHAKE =
    "GET " WEB_PATH " HTTP/1.1\r\n"
    "Host: " WEB_SERVER ":" WEB_PORT "\r\n"
    "Connection: Upgrade\r\n"
    "Upgrade: websocket\r\n"
    "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
    "Sec-WebSocket-Version: 13\r\n"
    "\r\n";

// WebSocket client context
typedef struct {
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;
} wss_client_t;

wss_client_t wss_client;

void wssClientInt() {
    char buf[1024];
    int ret, len;

    ESP_LOGI(TAG, "Initializing WebSocket Secure Client...");

    mbedtls_ssl_init(&wss_client.ssl);
    mbedtls_ctr_drbg_init(&wss_client.ctr_drbg);
    mbedtls_ssl_config_init(&wss_client.conf);
    mbedtls_entropy_init(&wss_client.entropy);

    ESP_LOGI(TAG, "Seeding the random number generator...");
    if ((ret = mbedtls_ctr_drbg_seed(&wss_client.ctr_drbg, mbedtls_entropy_func, &wss_client.entropy, NULL, 0)) != 0) {
        ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed failed: %d", ret);
        return;
    }

    ESP_LOGI(TAG, "Setting up SSL/TLS structure...");
    if ((ret = mbedtls_ssl_config_defaults(&wss_client.conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        ESP_LOGE(TAG, "mbedtls_ssl_config_defaults failed: %d", ret);
        return;
    }

    // Attach certificate bundle
    ret = esp_crt_bundle_attach(&wss_client.conf);
    if (ret < 0) {
        ESP_LOGE(TAG, "esp_crt_bundle_attach failed: -0x%x", -ret);
        return;
    }

    // Set hostname for TLS verification
    if ((ret = mbedtls_ssl_set_hostname(&wss_client.ssl, WEB_SERVER)) != 0) {
        ESP_LOGE(TAG, "mbedtls_ssl_set_hostname failed: -0x%x", -ret);
        return;
    }

    mbedtls_ssl_conf_authmode(&wss_client.conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_rng(&wss_client.conf, mbedtls_ctr_drbg_random, &wss_client.ctr_drbg);

    if ((ret = mbedtls_ssl_setup(&wss_client.ssl, &wss_client.conf)) != 0) {
        ESP_LOGE(TAG, "mbedtls_ssl_setup failed: -0x%x", -ret);
        return;
    }

    // Connect to WebSocket Server
    mbedtls_net_init(&wss_client.server_fd);
    ESP_LOGI(TAG, "Connecting to WebSocket server: %s:%s...", WEB_SERVER, WEB_PORT);
    if ((ret = mbedtls_net_connect(&wss_client.server_fd, WEB_SERVER, WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0) {
        ESP_LOGE(TAG, "mbedtls_net_connect failed: -0x%x", -ret);
        return;
    }

    ESP_LOGI(TAG, "Connected to WebSocket server.");

    mbedtls_ssl_set_bio(&wss_client.ssl, &wss_client.server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    ESP_LOGI(TAG, "Performing SSL/TLS handshake...");
    while ((ret = mbedtls_ssl_handshake(&wss_client.ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            ESP_LOGE(TAG, "mbedtls_ssl_handshake failed: -0x%x", -ret);
            return;
        }
    }

    ESP_LOGI(TAG, "TLS handshake successful! Sending WebSocket handshake request...");

    // Send WebSocket Upgrade Request
    size_t written_bytes = 0;
    do {
        ret = mbedtls_ssl_write(&wss_client.ssl, (const unsigned char *)WEBSOCKET_HANDSHAKE + written_bytes,
                                strlen(WEBSOCKET_HANDSHAKE) - written_bytes);
        if (ret >= 0) {
            written_bytes += ret;
        } else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ) {
            ESP_LOGE(TAG, "mbedtls_ssl_write failed: -0x%x", -ret);
            return;
        }
    } while (written_bytes < strlen(WEBSOCKET_HANDSHAKE));

    ESP_LOGI(TAG, "Waiting for WebSocket handshake response...");

    // Read WebSocket handshake response
    do {
        len = sizeof(buf) - 1;
        bzero(buf, sizeof(buf));
        ret = mbedtls_ssl_read(&wss_client.ssl, (unsigned char *)buf, len);
        if (ret < 0) {
            ESP_LOGE(TAG, "mbedtls_ssl_read failed: -0x%x", -ret);
            return;
        } else if (ret == 0) {
            ESP_LOGI(TAG, "Connection closed.");
            return;
        }

        buf[ret] = '\0';
        ESP_LOGI(TAG, "Received WebSocket handshake response: \n%s", buf);
    } while (1);

    ESP_LOGI(TAG, "WebSocket connection established! Ready to send/receive data.");
}

// Example function to send WebSocket messages
void wssSendMessage(const char *message) {
    size_t message_len = strlen(message);
    int ret = mbedtls_ssl_write(&wss_client.ssl, (const unsigned char *)message, message_len);
    if (ret < 0) {
        ESP_LOGE(TAG, "WebSocket send failed: -0x%x", -ret);
    } else {
        ESP_LOGI(TAG, "Sent WebSocket message: %s", message);
    }
}

// Example function to receive WebSocket messages
void wssReceiveMessage() {
    char buf[512];
    int ret = mbedtls_ssl_read(&wss_client.ssl, (unsigned char *)buf, sizeof(buf));
    if (ret < 0) {
        ESP_LOGE(TAG, "WebSocket receive failed: -0x%x", -ret);
    } else {
        buf[ret] = '\0';
        ESP_LOGI(TAG, "Received WebSocket message: %s", buf);
    }
}

void app_main() {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());  // Connect to Wi-Fi

    wssClientInt();  // Start WebSocket connection
    wssSendMessage("Hello from ESP32!");  // Send a message
    wssReceiveMessage();  // Receive messages
}
