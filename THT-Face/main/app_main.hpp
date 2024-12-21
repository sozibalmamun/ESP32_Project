#pragma once

#include <stdio.h>
#include <math.h>
#include "who_camera.h"
#include "who_human_face_recognition.hpp"
#include "who_lcd.h"
#include "who_button.h"
#include "event_logic.hpp"
#include "who_adc_button.h"

#include "Conectivity/blufi_example.h"

#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "timeLib.h"
#include "CloudDataHandle.h"

//--------fatfs-------------------
#include "FATFS/fs.h"
#include "gpioControl.h"


static const char *TAG = "app_main";

static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueLCDFrame = NULL;
static QueueHandle_t xQueueEventLogic = NULL;
static QueueHandle_t xQueueCloud = NULL;

extern uint8_t music;

void reInt(void);

