
#include "mynvs.h"
bool lisence =false;
bool dspTimeFormet;



void intTimeFormet(void){

    // dspTimeFormet;
        nvs_handle_t nvs_handle;
        esp_err_t  ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
        if (ret == ESP_OK) {
            int32_t saved_format = 0;
            ret = nvs_get_i32(nvs_handle, TIME_FORMAT_KEY, &saved_format);
            if (ret == ESP_OK) {
                dspTimeFormet = saved_format;
                // ESP_LOGI(TAG, "Retrieved time format: %s-hour", dspTimeFormet ? "12" : "24");
            } else {
                // ESP_LOGI(TAG, "Time format not found, defaulting to 12-hour.");
                dspTimeFormet = true; // Default to 12-hour format if not set
            }
            nvs_close(nvs_handle);
        }
}
void checkSubscription(void){
    
        nvs_handle_t nvs_handle;
        esp_err_t  ret = nvs_open(NVS_SUBCRIP_NAME_SPACE, NVS_READWRITE, &nvs_handle);
        if (ret == ESP_OK) {
            int32_t saved_format = 0;
            ret = nvs_get_i32(nvs_handle, NVS_SUBCRIP_NAME_SPACE_FORMAT_KEY, &saved_format);
            if (ret == ESP_OK) {
                lisence = saved_format;
            } else {
                lisence = false;
            }
            nvs_close(nvs_handle);
        }
}



    
void save_time_format(bool is_12_hour) {
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret == ESP_OK) {
        ret = nvs_set_i32(nvs_handle, TIME_FORMAT_KEY, is_12_hour);
        if (ret == ESP_OK) {
            nvs_commit(nvs_handle);
            // ESP_LOGI(TAG, "Time format saved as %s-hour", is_12_hour ? "12" : "24");
        }
        nvs_close(nvs_handle);
    }
}



// static const char *NVS_SUBCRIP_NAME_SPACE = "con";
// static const char *NVS_SUBCRIP_NAME_SPACE_FORMAT_KEY = "Sub";

void saveSubscription(bool enable) {
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_SUBCRIP_NAME_SPACE, NVS_READWRITE, &nvs_handle);
    if (ret == ESP_OK) {
        ret = nvs_set_i32(nvs_handle, NVS_SUBCRIP_NAME_SPACE_FORMAT_KEY, enable);
        if (ret == ESP_OK) {
            nvs_commit(nvs_handle);
        }
        nvs_close(nvs_handle);
    }
}



void welcomeMusic(bool enable) {

    if(checkMusicEnable()==enable) return;

    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_MUSIC_NAME_SPACE, NVS_READWRITE, &nvs_handle);
    if (ret == ESP_OK) {
        ret = nvs_set_i32(nvs_handle, NVS_MUSIC_NAME_SPACE_FORMAT_KEY, enable);
        if (ret == ESP_OK) {
            nvs_commit(nvs_handle);
        }
        nvs_close(nvs_handle);
    }
}


 bool checkMusicEnable(){

    nvs_handle_t nvs_handle;
    esp_err_t  ret = nvs_open(NVS_MUSIC_NAME_SPACE, NVS_READWRITE, &nvs_handle);
    int32_t musicPlay = 0;

    if (ret == ESP_OK) {
        ret = nvs_get_i32(nvs_handle, NVS_MUSIC_NAME_SPACE_FORMAT_KEY, &musicPlay);
        if (ret == ESP_OK) {
            // lisence = saved_format;
        } else {
            // lisence = false;
        }
        nvs_close(nvs_handle);
    }
    return musicPlay;
}