#include "who_camera.h"

#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "camera_task";

static QueueHandle_t xQueueFrameO = NULL;
TaskHandle_t cameraTaskHandler=NULL;

static void task_process_handler(void *arg)
{
    while (true)
    {
        camera_fb_t *frame = esp_camera_fb_get();
        if (frame)
            xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
    }
}

void register_camera(const pixformat_t pixel_fromat,
                     const framesize_t frame_size,
                     const uint8_t fb_count,
                     const QueueHandle_t frame_o)
{
    // ESP_LOGI(TAG, "Camera module is %s", CAMERA_MODULE_NAME);

#if CONFIG_CAMERA_MODULE_ESP_EYE || CONFIG_CAMERA_MODULE_ESP32_CAM_BOARD
    /* IO13, IO14 is designed for JTAG by default,
     * to use it as generalized input,
     * firstly declair it as pullup input */
    gpio_config_t conf;
    conf.mode = GPIO_MODE_INPUT;
    conf.pull_up_en = GPIO_PULLUP_ENABLE;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.intr_type = GPIO_INTR_DISABLE;
    conf.pin_bit_mask = 1LL << 13;
    gpio_config(&conf);
    conf.pin_bit_mask = 1LL << 14;
    gpio_config(&conf);
#endif

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = CAMERA_PIN_D0;
    config.pin_d1 = CAMERA_PIN_D1;
    config.pin_d2 = CAMERA_PIN_D2;
    config.pin_d3 = CAMERA_PIN_D3;
    config.pin_d4 = CAMERA_PIN_D4;
    config.pin_d5 = CAMERA_PIN_D5;
    config.pin_d6 = CAMERA_PIN_D6;
    config.pin_d7 = CAMERA_PIN_D7;
    config.pin_xclk = CAMERA_PIN_XCLK;
    config.pin_pclk = CAMERA_PIN_PCLK;
    config.pin_vsync = CAMERA_PIN_VSYNC;
    config.pin_href = CAMERA_PIN_HREF;
    config.pin_sscb_sda = CAMERA_PIN_SIOD;
    config.pin_sscb_scl = CAMERA_PIN_SIOC;
    config.pin_pwdn = CAMERA_PIN_PWDN;
    config.pin_reset = CAMERA_PIN_RESET;
    config.xclk_freq_hz = XCLK_FREQ_HZ;
    config.pixel_format = pixel_fromat;
    config.frame_size = frame_size;
    config.jpeg_quality = 12;// default: 12 camera quality range 0-63
    config.fb_count = fb_count;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        // ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        esp_restart();
        return;
    }




    sensor_t *s = esp_camera_sensor_get();

    // s->set_vflip(s, 1);      //  flip it back//change by sozib due to camera flip  int value was 1 for 2640 camera module
    // s->set_vflip(s, 1);         //  flip it back//change by sozib due to camera flip  int value was 1 for 5640 camera module
    
    // s->set_vflip(s, 0);         //  flip it back//change by sozib due to camera flip  int value was 0 for GC0308 camera module
    s->set_vflip(s, 1);         //  flip it back//change by sozib due to camera flip  int value was 1 for XR-3660-A4 camera module
    s->set_hmirror(s, 1);       //  miror camera change by sozib 

    // s->set_contrast(s, -1);     //  contrast    -2 - 2
    // s->set_brightness(s,-2);    //  brightness  -2 - 2
    // s->set_saturation(s, 2);    //  saturation  -2 - 2
    // s->set_sharpness(s, 2);     //  sharpness   -2 - 2 
    // s->set_wb_mode(s, 2);       //  white balance mode    0 - 4
    // s->set_whitebal(s, 1);
    // s->set_wpc(s, 1);
    // s->set_bpc(s, 1);



    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID)
    {
        s->set_brightness(s, 1);  //up the blightness just a bit
        s->set_saturation(s, -2); //lower the saturation
    }

    xQueueFrameO = frame_o;
    xTaskCreatePinnedToCore(task_process_handler, TAG, 3 * 1024, NULL,5 ,&cameraTaskHandler, 1); //2*1024 //5
}