/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


/****************************************************************************
* This is a demo for bluetooth config wifi connection to ap. You can config ESP32 to connect a softap
* or config ESP32 as a softap to be connected by other device. APP can be downloaded from github
* android source code: https://github.com/EspressifApp/EspBlufi
* iOS source code: https://github.com/EspressifApp/EspBlufiForiOS
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_blufi_api.h"
#include "blufi_example.h"

#include "esp_blufi.h"
#include "esp_bt_main.h"
#include "esp_netif.h"  // For netif functions
#include "math.h"
static const char *TAG = "BLUFI_HANDLER";




#define EXAMPLE_WIFI_CONNECTION_MAXIMUM_RETRY 2
#define EXAMPLE_INVALID_REASON                255
#define EXAMPLE_INVALID_RSSI                  -128

static void example_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);

#define WIFI_LIST_NUM   10

static wifi_config_t sta_config;
static wifi_config_t ap_config;
esp_netif_t *sta_netif= NULL;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

static uint8_t example_wifi_retry = 0;

/* store the station info for send back to phone */
uint8_t networkStatus=0;
static bool gl_sta_got_ip = false;
bool ble_is_connected = false;
static uint8_t gl_sta_bssid[6];
static uint8_t gl_sta_ssid[32];
static int gl_sta_ssid_len;
// static wifi_sta_list_t gl_sta_list;
static bool gl_sta_is_connecting = false;
static esp_blufi_extra_info_t gl_sta_conn_info;



uint64_t generate_unique_id(void)
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    uint64_t uniqueId = ((uint64_t)mac[0] << 40) | ((uint64_t)mac[1] << 32) | ((uint64_t)mac[2] << 24) |
                         ((uint64_t)mac[3] << 16) | ((uint64_t)mac[4] << 8) | mac[5];

    return uniqueId;

}


static void example_record_wifi_conn_info(int rssi, uint8_t reason)
{
    memset(&gl_sta_conn_info, 0, sizeof(esp_blufi_extra_info_t));
    if (gl_sta_is_connecting) {
        gl_sta_conn_info.sta_max_conn_retry_set = true;
        gl_sta_conn_info.sta_max_conn_retry = EXAMPLE_WIFI_CONNECTION_MAXIMUM_RETRY;
    } else {
        gl_sta_conn_info.sta_conn_rssi_set = true;
        gl_sta_conn_info.sta_conn_rssi = rssi;
        gl_sta_conn_info.sta_conn_end_reason_set = true;
        gl_sta_conn_info.sta_conn_end_reason = reason;
    }
}

static void example_wifi_connect(void)
{
    example_wifi_retry = 0;
    gl_sta_is_connecting = (esp_wifi_connect() == ESP_OK);
    example_record_wifi_conn_info(EXAMPLE_INVALID_RSSI, EXAMPLE_INVALID_REASON);
}

static bool example_wifi_reconnect(void)
{
    bool ret;
    if (gl_sta_is_connecting && example_wifi_retry++ < EXAMPLE_WIFI_CONNECTION_MAXIMUM_RETRY) {
        // BLUFI_INFO("BLUFI WiFi starts reconnection\n");
        gl_sta_is_connecting = (esp_wifi_connect() == ESP_OK);
        example_record_wifi_conn_info(EXAMPLE_INVALID_RSSI, EXAMPLE_INVALID_REASON);
        ret = true;
    } else {
        ret = false;
    }
    return ret;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    wifi_event_sta_connected_t *event;
    // wifi_event_sta_disconnected_t *disconnected_event;

    switch (event_id) {
    case WIFI_EVENT_STA_START:////ok

        example_wifi_connect();
        // printf("WIFI CONNECTING....\n");

        break;

    case WIFI_EVENT_STA_CONNECTED:{
        printf("WiFi CONNECTED\n");
        gl_sta_is_connecting = false;

        vTaskDelay(100);
        // gl_sta_is_connecting = false;
        event = (wifi_event_sta_connected_t*) event_data;
        memcpy(gl_sta_bssid, event->bssid, 6);
        memcpy(gl_sta_ssid, event->ssid, event->ssid_len);
        gl_sta_ssid_len = event->ssid_len;

        esp_blufi_adv_stop();
        blufi_security_init();

        break;
    }
    case WIFI_EVENT_STA_DISCONNECTED:{//ok

 
        printf("Retrying to Connect...\n");
        example_wifi_connect();

        blufi_security_deinit();
        blufiAddStart();



        break;
    }

    default:
        break;
    }
    return;
}

static void initialise_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();

    // ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create event loop if not created already
    if (esp_event_loop_create_default() != ESP_OK) {
        // ESP_LOGW(TAG, "Event loop already created.");
    }

    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    // ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    example_record_wifi_conn_info(EXAMPLE_INVALID_RSSI, EXAMPLE_INVALID_REASON);

    ESP_ERROR_CHECK( esp_wifi_start());

    // esp_wifi_set_max_tx_power(8);  // Set power level to 8 (2 dBm)

}

static esp_blufi_callbacks_t example_callbacks = {
    .event_cb = example_event_callback,
    .negotiate_data_handler = blufi_dh_negotiate_data_handler,
    .encrypt_func = blufi_aes_encrypt,
    .decrypt_func = blufi_aes_decrypt,
    .checksum_func = blufi_crc_checksum,
};

static void example_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param)
{
    /* actually, should post to blufi_task handle the procedure,
     * now, as a example, we do it more simply */
    switch (event) {
    case ESP_BLUFI_EVENT_INIT_FINISH:
        // BLUFI_INFO("BLUFI init finish\n");
        blufi_security_deinit();
        blufiAddStart();

        break;
    case ESP_BLUFI_EVENT_DEINIT_FINISH:
        // BLUFI_INFO("BLUFI deinit finish\n");
        break;
    case ESP_BLUFI_EVENT_BLE_CONNECT:
        BLUFI_INFO("BLUFI ble connect\n");
        ble_is_connected = true;
        esp_blufi_adv_stop();
        blufi_security_init();
        break;
    case ESP_BLUFI_EVENT_BLE_DISCONNECT:
        BLUFI_INFO("BLUFI ble disconnect\n");
        ble_is_connected = false;
        blufi_security_deinit();
        blufiAddStart();

        break;


	case ESP_BLUFI_EVENT_RECV_STA_SSID:{//ok

        strncpy((char *)sta_config.sta.ssid, (char *)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
        sta_config.sta.ssid[param->sta_ssid.ssid_len] = '\0';
        esp_wifi_set_config(WIFI_IF_STA, &sta_config);


        // char ssid[param->sta_ssid.ssid_len + 1];
        // strncpy(ssid, (char *)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
        // ssid[param->sta_ssid.ssid_len] = '\0'; // Null-terminate the SSID string

        // BLUFI_INFO("Received STA SSID: %s\n", ssid);
        break;
    }
	case ESP_BLUFI_EVENT_RECV_STA_PASSWD:{//ok

        strncpy((char *)sta_config.sta.password, (char *)param->sta_passwd.passwd, param->sta_passwd.passwd_len);
        sta_config.sta.password[param->sta_passwd.passwd_len] = '\0';
        esp_wifi_set_config(WIFI_IF_STA, &sta_config);

        // char password[param->sta_passwd.passwd_len + 1];
        // strncpy(password, (char *)param->sta_passwd.passwd, param->sta_passwd.passwd_len);
        // password[param->sta_passwd.passwd_len] = '\0'; // Null-terminate the password string

        // BLUFI_INFO("Received STA PASSWORD: %s\n", password);
        example_wifi_connect();



        break;
 
    }

        case ESP_BLUFI_EVENT_RECV_CUSTOM_DATA:{
        BLUFI_INFO("Recv Custom Data %d\n", param->custom_data.data_len);
        esp_log_buffer_hex("Custom Data", param->custom_data.data, param->custom_data.data_len);

        // Print the received data as a string
        char received_data_str[param->custom_data.data_len + 1];
        memcpy(received_data_str, param->custom_data.data, param->custom_data.data_len);
        received_data_str[param->custom_data.data_len] = '\0'; // Null-terminate the string

        printf("Received Custom Data: %s\n", received_data_str);


        break;
    }
    default:
        break;
    }
}
void blufiAddStart(void){
    char tempFrame[14]="sdfg" ;
    esp_ble_gap_set_device_name(tempFrame);
    esp_blufi_adv_start();
}
void bluFiStart(void)
{
    esp_err_t ret;

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    initialise_wifi();

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        // BLUFI_ERROR("%s initialize bt controller failed: %s\n", __func__, esp_err_to_name(ret));
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        // BLUFI_ERROR("%s enable bt controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_blufi_host_and_cb_init(&example_callbacks);
    if (ret) {
        // BLUFI_ERROR("%s initialise failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

}

void deinitBlufi(void) {
    printf("\nBLUFI: Disabling...");


    // networkStatus=0;
    gl_sta_got_ip = false;
    ble_is_connected = false;
    gl_sta_is_connecting = false;

    // Stop and deinitialize Wi-Fi
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_netif_destroy_default_wifi(sta_netif);  // Remove the default Wi-Fi netif
    // ESP_LOGI(TAG, "Wi-Fi deinitialized and netif destroyed.");

    // Deinitialize BLUFI profile
    esp_blufi_profile_deinit();
    // ESP_LOGI(TAG, "BLUFI profile deinitialized.");

    // Delete the default event loop
    esp_event_loop_delete_default();
    // ESP_LOGI(TAG, "Event loop deleted.");

    // Disable and deinitialize Bluedroid stack
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    // ESP_LOGI(TAG, "Bluedroid stack deinitialized.");

    // Disable and deinitialize Bluetooth controller
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
    // ESP_LOGI(TAG, "Bluetooth controller deinitialized.");
}

uint8_t wifi_rssi_to_percentage(int32_t rssi) {

     if (rssi <= -120) {
        return 0;    // 0% for very weak signals or below -120 dBm
    } else if (rssi >= -35) {
        return 100;  // 100% for strong signals at or above -35 dBm
    } else {
        // Linear mapping from -120 dBm to -35 dBm to 0-100%
        return (uint8_t)((rssi + 120) * 100 / 85);
    }

}