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

wl_handle_t s_wl_handle;

#define  FACE_DIRECTORIES "/fatfs/faces"
#define  LOG_DIRECTORIES  "/fatfs/attendance"


#ifdef __cplusplus
extern "C" {
#endif

void init_fatfs();
void create_directories();

extern void save_face_data(uint32_t person_id, const char* name, uint32_t image_width, uint32_t image_length, const uint8_t* image_data);
extern void read_face_data(uint32_t person_id);
extern void delete_face_data(uint32_t person_id);
extern void wright_log_attendance(uint32_t person_id, const char* timestamp);
extern void read_attendance_log(const char* date);
extern void delete_attendance_log(const char* date);
extern void print_memory_status(void);





#ifdef __cplusplus
}
#endif


#endif