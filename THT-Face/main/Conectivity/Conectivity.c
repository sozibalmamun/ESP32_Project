#include "Conectivity.h"

#define     TAG_WI_FI       "Wifi Debug"
#define     TAG_FS          "FS Debug"
#define     DEVICE_NAME         "THT-Face"



const char *ssid = "myssid";
const char *pass = "mypassword";


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
        ESP_LOGE(TAG_WI_FI, "Failed to get Wi-Fi RSSI");
        return 0;
    }
}

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

// void bt_init(void)
// {
//     esp_err_t ret;
//     // Initialize NVS for Bluetooth
//     ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);

//     // Initialize the BT controller to allocate task and other resources.
//     esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
//     ret = esp_bt_controller_init(&bt_cfg);
//     if (ret) {
//         ESP_LOGE(BT_TAG, "%s initialize controller failed\n", __func__);
//         return;
//     }

//     // Enable BT controller
//     ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
//     if (ret) {
//         ESP_LOGE(BT_TAG, "%s enable controller failed\n", __func__);
//         return;
//     }

//     // Initialize and enable Bluedroid stack
//     ret = esp_bluedroid_init();
//     if (ret) {
//         ESP_LOGE(BT_TAG, "%s initialize bluedroid failed\n", __func__);
//         return;
//     }

//     ret = esp_bluedroid_enable();
//     if (ret) {
//         ESP_LOGE(BT_TAG, "%s enable bluedroid failed\n", __func__);
//         return;
//     }
// }

// void gatts_profile_event_handler(esp_gatts_cb_event_t event,
//                                  esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
//     switch (event) {
//         case ESP_GATTS_WRITE_EVT: {
//             if (param->write.handle == /* handle of your characteristic */) {
//                 // Assuming credentials are sent in the format "SSID:PASSWORD"
//                 char received_data[128];
//                 memcpy(received_data, param->write.value, param->write.len);
//                 received_data[param->write.len] = '\0';

//                 char *ssid = strtok(received_data, ":");
//                 char *password = strtok(NULL, ":");

//                 if (ssid && password) {
//                     ESP_LOGI(GATTS_TAG, "Received SSID: %s, Password: %s", ssid, password);
//                     // Connect to WiFi with the received credentials
//                     wifi_init_sta(ssid, password);
//                 }
//             }
//             break;
//         }
//         // Handle other events
//     }
// }
