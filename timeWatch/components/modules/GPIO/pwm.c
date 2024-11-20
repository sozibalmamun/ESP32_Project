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
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 8192);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

    // Optional: Use fading
    ledc_fade_func_install(0);  // Install the fade function
    ledc_set_fade_time_and_start(ledc_channel.speed_mode, ledc_channel.channel, 8192, 1000, LEDC_FADE_NO_WAIT);
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