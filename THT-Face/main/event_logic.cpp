#include <stdio.h>
#include "event_logic.hpp"
#include "who_button.h"
#include "who_human_face_recognition.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

typedef enum {
    MENU = 1,
    PLAY,
    UP,
    DOWN
} key_name_t;

static QueueHandle_t xQueueEventO = NULL;
key_state_t key_state ;    

static recognizer_state_t recognizer_state;

extern volatile uint8_t sleepEnable;
extern volatile TickType_t sleepTimeOut; 
// extern uint8_t volatile eventState;

void event_generate(void *arg) {

    while (1) {



//     KEY_SHORT_PRESS = 1, recg 1
//     KEY_LONG_PRESS,      enroll 2
//     KEY_DOUBLE_CLICK,    delete 3


        switch (key_state) {

            case KEY_SHORT_PRESS:
                recognizer_state = ENROLING;

                break;

            case KEY_LONG_PRESS:
                // recognizer_state = ENROLL;

                break;

            case KEY_DOUBLE_CLICK:
                recognizer_state = DELETE;

                break;

            default:
                recognizer_state = DETECT;
                break;
        }

        xQueueSend(xQueueEventO, &recognizer_state, portMAX_DELAY); // Send the recognizer state to the output queue

        // vTaskDelay(pdMS_TO_TICKS(10)); // Add a small delay to prevent task hogging CPU
    }
}

void register_event(const QueueHandle_t event_o) {
    xQueueEventO = event_o;
    xTaskCreatePinnedToCore(event_generate, "event_logic_task", 1024, NULL, 1, NULL, 0);
}
