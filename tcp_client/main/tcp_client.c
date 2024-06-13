#include <stdio.h>           // For basic printf commands
#include <string.h>          // For handling strings
#include "freertos/FreeRTOS.h"  // For delay, mutexes, semaphores, RTOS operations
#include "esp_system.h"      // For ESP init functions, esp_err_t
#include "esp_wifi.h"        // For esp_wifi_init functions and Wi-Fi operations
#include "esp_log.h"         // For showing logs
#include "esp_event.h"       // For Wi-Fi events
#include "nvs_flash.h"       // Non-volatile storage
#include "lwip/err.h"        // Light weight IP packets error handling
#include "lwip/sys.h"        // System applications for lightweight IP apps
#include "nvs.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"

// #include "fs.h"

#define WIFI_SSID_KEY "wifi_ssid"
#define WIFI_PASS_KEY "wifi_pass"

const char *ssid = "SOZIB";
const char *pass = "123456789";
const char *TAG_WI_FI = "Wifi Debug";
const char *TAG_FS   = "FS Debug";


void save_wifi_info(const char* ssid, const char* pass);
void read_wifi_info(char* ssid, char* pass);
esp_err_t read_wifi_credentials(char *ssid, size_t ssid_len, char *pass, size_t pass_len) ;
esp_err_t save_wifi_credentials(const char *ssid, const char *pass) ;
void print_hostname();
void set_and_print_hostname();

static const char *TAG = "example";

esp_err_t save_wifi_credentials(const char *ssid, const char *pass) {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(TAG, "NVS handle opened successfully");

        // Write SSID
        err = nvs_set_str(nvs_handle, WIFI_SSID_KEY, ssid);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to write SSID to NVS (%s)", esp_err_to_name(err));
        }

        // Write Password
        err = nvs_set_str(nvs_handle, WIFI_PASS_KEY, pass);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to write Password to NVS (%s)", esp_err_to_name(err));
        }

        // Commit written value
        err = nvs_commit(nvs_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to commit written value to NVS (%s)", esp_err_to_name(err));
        }

        // Close NVS handle
        nvs_close(nvs_handle);
    }

    return err;
}

esp_err_t read_wifi_credentials(char *ssid, size_t ssid_len, char *pass, size_t pass_len) {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    } else {
        ESP_LOGI(TAG, "NVS handle opened successfully");

        // Read SSID
        err = nvs_get_str(nvs_handle, WIFI_SSID_KEY, ssid, &ssid_len);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read SSID from NVS (%s)", esp_err_to_name(err));
        }

        // Read Password
        err = nvs_get_str(nvs_handle, WIFI_PASS_KEY, pass, &pass_len);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read Password from NVS (%s)", esp_err_to_name(err));
        }

        // Close NVS handle
        nvs_close(nvs_handle);
    }

    return err;
}

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
        set_and_print_hostname("THTFace");

    }
}

void wifi_connection() {

    char ssid[32];
    char pass[64];
    size_t ssid_len = sizeof(ssid);
    size_t pass_len = sizeof(pass);


    // set host namae // espressif
   // esp_netif_t* netif = esp_netif_create_default_wifi_sta();
  //  esp_netif_set_hostname(netif, "THTFace");

    // Read Wi-Fi credentials from NVS
    esp_err_t err = read_wifi_credentials(ssid, ssid_len, pass, pass_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read Wi-Fi credentials from NVS. Using default credentials.");
        strcpy(ssid, "SOZIB");
        strcpy(pass, "123456789");
        save_wifi_credentials(ssid, pass);
    }

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
    


    // Wi-Fi Start Phase
    esp_wifi_start();
    esp_wifi_set_mode(WIFI_MODE_STA);

    // Wi-Fi Connect Phase
    esp_wifi_connect();
}

void init_spiffs() {

    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }



 
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(conf.partition_label);
        return;
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }


}

void save_wifi_info(const char* ssid, const char* pass) {
    FILE* f = fopen("/spiffs/wifi_info.txt", "w");
    if (f == NULL) {
        ESP_LOGE(TAG_FS, "Failed to open file for writing");
        return;
    }
    fprintf(f, "%s\n%s\n", ssid, pass);
    fclose(f);
    ESP_LOGI(TAG_FS, "Wi-Fi info saved");
}

void read_wifi_info(char* ssid, char* pass) {
    FILE* f = fopen("/spiffs/wifi_info.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG_FS, "Failed to open file for reading");
        return;
    }
    fgets(ssid, 32, f);  // Assuming SSID length is 32
    ssid[strcspn(ssid, "\n")] = 0;  // Remove newline character
    fgets(pass, 64, f);  // Assuming password length is 64
    pass[strcspn(pass, "\n")] = 0;  // Remove newline character
    fclose(f);
    ESP_LOGI(TAG_FS, "Wi-Fi info read: SSID: %s, PASS: %s", ssid, pass);
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



void app_main(void) {
    nvs_flash_init();
   // init_spiffs();
    wifi_connection();
}
