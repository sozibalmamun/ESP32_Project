
#include "tcp_client.h"

#define WIFI_SSID_KEY "wifi_ssid"
#define WIFI_PASS_KEY "wifi_pass"
#define DEVICE_NAME "THT-Face"
const char *ssid = "SOZIB";
const char *pass = "123456789";
const char *TAG_WI_FI = "Wifi Debug";
const char *TAG_FS   = "FS Debug";

static const char *TAG = "example";


static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_id == WIFI_EVENT_STA_START) {
        printf("WIFI CONNECTING....\n");
    } else if (event_id == WIFI_EVENT_STA_CONNECTED) {
        printf("WiFi CONNECTED\n");
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        printf("WiFi lost connection\n");
        esp_wifi_connect();
        printf("Retrying to Connect...\n");
    } else if (event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG_WI_FI, "WiFi got IP: " IPSTR, IP2STR(&event->ip_info.ip));

        uint8_t mac[6];
        esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
        ESP_LOGI(TAG_WI_FI, "MAC address: %02x:%02x:%02x:%02x:%02x:%02x",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        // Read and log the hostname
        print_hostname();
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
    
    set_and_print_hostname(DEVICE_NAME);

    // Wi-Fi Start Phase
    esp_wifi_start();
    esp_wifi_set_mode(WIFI_MODE_STA);

    // Wi-Fi Connect Phase
    esp_wifi_connect();
}



void print_hostname() {
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif == NULL) {
        ESP_LOGE(TAG, "Failed to get netif handle");
        return;
    }

    const char *hostname;
    esp_err_t err = esp_netif_get_hostname(netif, &hostname);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Hostname: %s", hostname);
    } else {
        ESP_LOGE(TAG, "Failed to get hostname: %s", esp_err_to_name(err));
    }
}
// Function to set and print the hostname
void set_and_print_hostname(char *hostName) {

    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif == NULL) {
        ESP_LOGE(TAG, "Failed to get netif handle");
        return;
    }

    // Set the hostname to "THTFace"
    const char *new_hostname = hostName;
    esp_err_t err = esp_netif_set_hostname(netif, new_hostname);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Hostname set to: %s", new_hostname);
    } else {
        ESP_LOGE(TAG, "Failed to set hostname: %s", esp_err_to_name(err));
        return;
    }

    // Retrieve and log the hostname to verify
    const char *hostname;
    err = esp_netif_get_hostname(netif, &hostname);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Hostname: %s", hostname);
    } else {
        ESP_LOGE(TAG, "Failed to get hostname: %s", esp_err_to_name(err));
    }
}



// void app_main(void) {
//     nvs_flash_init();
//     wifi_connection();
// }
