// #pragma once

#ifndef FS_H
#define FS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <dirent.h>  // For directory traversal
#include <stdint.h>  // For standard integer types
#include <stdbool.h> // For boolean type
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
#include "CloudDataHandle.h"



static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
extern bool dataAvailable;
extern bool dspTimeFormet;


// Define the path where the partition is mounted
#define BASE_PATH "/fatfs"
#define MOUNT_POINT "/fatfs"




#ifdef __cplusplus
extern "C" {
#endif

esp_err_t init_fatfs(void);
esp_err_t formatfatfs();


extern void save_face_data(uint16_t person_id, const char* name, uint8_t image_width, uint8_t image_height, const uint8_t* image_data, const char* directory);
extern bool syncFace(const camera_fb_t *src, imageData_t **person);
extern bool delete_face_data(uint16_t person_id , const char * directory);
extern void write_log_attendance(uint16_t person_id,  uint8_t* timestamp);
extern void print_memory_status(void);
extern void process_attendance_files(void);
extern bool process_and_send_faces(uint16_t id);
extern bool imagesent(uint8_t *buff, uint16_t buffLen, uint8_t h, uint8_t w ,char* name,uint16_t id);
extern bool display_faces( camera_fb_t *buff);
extern void drawImage_u8(uint16_t x_offset, uint8_t y_offset, uint8_t width, uint8_t height, uint8_t *image, camera_fb_t *buff);
extern void scaleAndDisplayImageInFrame(uint8_t *src_image, uint8_t src_width, uint8_t src_height, camera_fb_t *dst_buff, uint8_t pos_x, uint8_t pos_y);
extern bool pendingData();
extern bool sendToWss(uint8_t *buff, size_t buffLen);
extern uint64_t generate_unique_id(void);

bool sendFilePath(const char *file_path);
void format_fatfs(void);
void create_directories();
void delete_all_directories(const char* path);








#ifdef __cplusplus
}
#endif


#endif