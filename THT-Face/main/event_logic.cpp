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
static key_state_t key_state;
static recognizer_state_t recognizer_state;

extern volatile uint8_t sleepEnable;
extern volatile TickType_t sleepTimeOut; 
extern volatile uint8_t eventState=0; // Ensure this is declared and managed correctly elsewhere

void event_generate(void *arg) {
    while (1) {
        // No longer using xQueueReceive, directly using eventState
        switch (eventState) {
            case KEY_SHORT_PRESS:
                // recognizer_state = RECOGNIZE;
                // sleepTimeOut = xTaskGetTickCount(); // Update timeout to prevent sleep
                // sleepEnable = false;
                break;

            case KEY_LONG_PRESS:
                recognizer_state = ENROLL;
                break;

            case KEY_DOUBLE_CLICK:
                recognizer_state = DELETE;
                break;

            default:
                recognizer_state = RECOGNIZE;
                break;
        }

        xQueueSend(xQueueEventO, &recognizer_state, portMAX_DELAY); // Send the recognizer state to the output queue
        vTaskDelay(pdMS_TO_TICKS(10)); // Add a small delay to prevent task hogging CPU
    }
}

void register_event(const QueueHandle_t event_o) {
    xQueueEventO = event_o;
    xTaskCreatePinnedToCore(event_generate, "event_logic_task", 1024, NULL, 1, NULL, 0);
}
