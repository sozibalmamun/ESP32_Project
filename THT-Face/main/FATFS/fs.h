// #pragma once

#ifndef FS_H
#define FS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
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

#include "handleQrCode.h"
#include "timeLib.h"

#include "globalScope.h"

static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

#define  FACE_DIRECTORIES "/fatfs/faces"
#define  ATTENDANCE_DIR  "/fatfs/log"


// Define the path where the partition is mounted
#define BASE_PATH "/fatfs"
#define MOUNT_POINT "/fatfs"




#ifdef __cplusplus
extern "C" {
#endif

esp_err_t init_fatfs(void);
void create_directories();
bool sendFilePath(const char *file_path);
void delete_all_directories(const char* path);


extern void save_face_data(uint32_t person_id, const char* name, uint32_t image_width, uint32_t image_length, const uint8_t* image_data);
extern void read_face_data(uint32_t person_id);
extern void delete_face_data(uint32_t person_id);
extern void write_log_attendance(uint32_t person_id,  char* timestamp);
extern void read_attendance_log(const char* date);
extern void delete_attendance_log(const char* date);
extern void print_memory_status(void);
extern void format_fatfs(void);
extern void process_attendance_files(void);
extern bool stompSend(char * buff, char* topic);
extern uint64_t generate_unique_id(void);







#ifdef __cplusplus
}
#endif


#endif