// #pragma once

#ifndef FS_H
#define FS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "wear_levelling.h"
#include "esp_vfs.h"
#include "esp_log.h"
#include "sys/stat.h"

#include "esp_flash.h"
#include "esp_flash_spi_init.h"
#include "esp_partition.h"
#include "esp_system.h"
#include <errno.h>

static wl_handle_t s_wl_handle;



#ifdef __cplusplus
extern "C" {
#endif

void init_fatfs();
void create_directories();
void save_face_data(uint32_t person_id, const char* name, uint32_t image_width, uint32_t image_length, const uint8_t* image_data);
void read_face_data(uint32_t person_id);
void delete_face_data(uint32_t person_id);
void log_attendance(uint32_t person_id, const char* timestamp);
void read_attendance_log(const char* date);
void delete_attendance_log(const char* date);





#ifdef __cplusplus
}
#endif


#endif