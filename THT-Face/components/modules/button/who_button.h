#pragma once
#include "esp_event_loop.h"
#include "driver/gpio.h"
#include "esp_log.h"

typedef enum
{
    KEY_IDLE,
    KEY_SHORT_PRESS ,
    KEY_LONG_PRESS,
    KEY_DOUBLE_CLICK,
    KEY_SYNC,
    KEY_DUMP,
} key_state_t;

extern  volatile uint8_t CmdEvent;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
 * @brief initialize gpio button
 * 
 * @param key_io_num the gpio number of the button
 * @param key_state_o the queue to send the button state
 */
    void register_button(const gpio_num_t key_io_num, const QueueHandle_t key_state_o);

#ifdef __cplusplus
}
#endif