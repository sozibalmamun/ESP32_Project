#include "pwm.h"


void PwmInt( gpio_num_t pinNo ) {

    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,   // Low-speed mode
        .duty_resolution = LEDC_TIMER_13_BIT,      // 13-bit duty resolution
        .timer_num       = LEDC_TIMER_0,           // Timer 0
        .freq_hz         = 5000,                   // 5kHz PWM frequency
        .clk_cfg         = LEDC_AUTO_CLK           // Auto select clock
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel={

    // Configure LEDC channel for GPIO 14
        .gpio_num   = pinNo,                // GPIO pin for PWM output
        .speed_mode = LEDC_LOW_SPEED_MODE,        // Low-speed mode
        .channel    = LEDC_CHANNEL_0,             // Channel: 0
        .intr_type  = LEDC_INTR_DISABLE,          // Disable fade interrupt
        .timer_sel  = LEDC_TIMER_0,               // Select Timer 0
        .duty       = 0,                          // Initial duty cycle: 0
        .hpoint     = 0,                          // Hpoint: 0
        .flags.output_invert = 0                 // Disable output inversion
    };


    // Initialize the channel
    ledc_channel_config(&ledc_channel);

    // Set initial duty cycle to 50% (for 13-bit resolution, this is 4096 out of 8192)
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 0);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

    // Optional: Use fading
    ledc_fade_func_install(0);  // Install the fade function
    ledc_set_fade_time_and_start(ledc_channel.speed_mode, ledc_channel.channel, 0, 1000, LEDC_FADE_NO_WAIT);
    // for (int duty = 8192; duty >= 0; duty -= 64) {
    //     ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, duty);
    //     ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
    //     vTaskDelay(100 / portTICK_PERIOD_MS);     // Delay to see the dimming effect


    // }

    printf("pwm int done\n");
}


void brightness(bool sleep){
    
    ledc_channel_config_t ledc_channel={

        .speed_mode = LEDC_LOW_SPEED_MODE,        // Low-speed mode
        .channel    = LEDC_CHANNEL_0,             // Channel: 0

    };

    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, sleep?BRIGHTNESS(SLEEP_LCD): BRIGHTNESS(WAKE_LCD));//8192
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

}




#define BUZZER_GPIO 8         // GPIO 8 for the buzzer
#define BUZZER_FREQ 1000      // Frequency in Hz (lower frequency for testing)
#define BUZZER_DUTY 4096      // Duty cycle (50% of 8192, 13-bit resolution)
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_7


void buzzer_init() {
    // Configure LEDC timer
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_TIMER_13_BIT,  // 13-bit resolution (8192 steps)
        .freq_hz = BUZZER_FREQ,
        .clk_cfg = LEDC_USE_APB_CLK,           // Use the APB clock
    };
    esp_err_t res = ledc_timer_config(&timer_conf);
    if (res != ESP_OK) {
        ESP_LOGE("BUZZER", "Failed to configure LEDC timer: %s", esp_err_to_name(res));
    }

    // Configure LEDC channel
    ledc_channel_config_t channel_conf = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .gpio_num = BUZZER_GPIO,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = BUZZER_DUTY,  // 50% duty cycle
        .hpoint = 0,
    };
    res = ledc_channel_config(&channel_conf);
    if (res != ESP_OK) {
        ESP_LOGE("BUZZER", "Failed to configure LEDC channel: %s", esp_err_to_name(res));
    }
}

void buzzer_play(uint16_t freq) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, freq);  // Set duty to 0 (stop)
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    ESP_LOGI("BUZZER", "Buzzer ON");
}

void play_music(uint16_t *sequence, size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (i % 2 == 0) {
            // Even index: frequency
            buzzer_play(sequence[i]*3);
        } else {
            // Odd index: delay
            vTaskDelay(sequence[i] / portTICK_PERIOD_MS); // Convert to ms
        }
    }
}
