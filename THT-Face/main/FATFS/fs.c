
#include "fs.h"

static const char *TAG = "FAT";
#define PARTITION_LABEL "storage"  // Replace "storage" with the actual partition label if it's different
// Function to initialize and mount FAT filesystem
esp_err_t init_fatfs(void) {
    esp_err_t ret;
    const esp_vfs_fat_mount_config_t mount_config = {
        .max_files = 10,
        .format_if_mount_failed = true,
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };

    // Mount FATFS partition
    ret = esp_vfs_fat_spiflash_mount(MOUNT_POINT, "storage", &mount_config, &s_wl_handle);
    if (ret != ESP_OK) {
        ESP_LOGE("init_fatfs", "Failed to mount FATFS (%s)", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI("init_fatfs", "FATFS mounted successfully");


    return ESP_OK;
}

void create_directories(void) {

    vTaskDelay(pdMS_TO_TICKS(30));

    struct stat st;
    if (stat(BASE_PATH "/log", &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            ESP_LOGI("FAT", "Directory /log already exists");
        } else {
            ESP_LOGE("FAT", "/log exists but is not a directory");
        }
    } else {

        // If the directory does not exist, try to create it
        int res = mkdir(BASE_PATH "/log", 0777);
        if (res != 0 && errno != EEXIST) {

            ESP_LOGE("FAT", "Failed to create directory: %s", BASE_PATH "/attendance");
            // esp_restart();

        } else {
            ESP_LOGI("FAT", "Directory /attendance created");
        }
    }

    vTaskDelay(pdMS_TO_TICKS(30));

    if (stat(BASE_PATH "/faces", &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            ESP_LOGI("FAT", "Directory /faces already exists");
        } else {
            ESP_LOGE("FAT", "/faces exists but is not a directory");
        }
    } else {
        // If the directory does not exist, try to create it
        int res = mkdir(BASE_PATH "/faces", 0777);
        if (res != 0 && errno != EEXIST) {
            ESP_LOGE("FAT", "Failed to create directory: %s", BASE_PATH "/faces");
            // esp_restart();

        } else {
            ESP_LOGI("FAT", "Directory /faces created");
        }
    }

    vTaskDelay(pdMS_TO_TICKS(50));

    if (stat(BASE_PATH "/sync", &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            ESP_LOGI("FAT", "Directory /sync already exists");
        } else {
            ESP_LOGE("FAT", "/sync exists but is not a directory");
        }
    } else {
        // If the directory does not exist, try to create it
        int res = mkdir(BASE_PATH "/sync", 0777);
        if (res != 0 && errno != EEXIST) {
            ESP_LOGE("FAT", "Failed to create directory: %s", BASE_PATH "/sync");
            // esp_restart();

        } else {
            ESP_LOGI("FAT", "Directory /sync created");
        }
    }

    vTaskDelay(pdMS_TO_TICKS(30));


}

void print_memory_status() {
    // FATFS *fs;
    // DWORD fre_clust, fre_sect, tot_sect;

    // if (f_getfree("/storage", &fre_clust, &fs) == FR_OK) {
    //     tot_sect = (fs->n_fatent - 2) * fs->csize * 512;
    //     fre_sect = fre_clust * fs->csize * 512;

    //     ESP_LOGI("FAT ", "Total Space: %" PRIu32 " bytes", (uint32_t)tot_sect);
    //     ESP_LOGI("FAT ", "Free  Space: %" PRIu32 " bytes", (uint32_t)fre_sect);
    //     ESP_LOGE("FAT ", "Used  Space: %" PRIu32 " bytes", (uint32_t)(tot_sect - fre_sect));

    // } else {
    //     ESP_LOGE("FAT", "Failed to get FATFS free space info");
    // }


    FATFS *fs;
    DWORD fre_clust, fre_sect, tot_sect;

    // Get the FATFS file system object (fs)
    if (f_getfree("0:", &fre_clust, &fs) != FR_OK) {
        ESP_LOGE(TAG, "Failed to get FATFS free space information");
        return;
    }

    // Get total sectors and free sectors
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;

    // Calculate total space and free space (in bytes)
    uint64_t total_space = tot_sect * fs->ssize;
    uint64_t free_space = fre_sect * fs->ssize;

    // Log the available and total space
    ESP_LOGW(TAG, "Total space: %llu bytes", total_space);
    ESP_LOGE(TAG, "free space : %llu bytes\n", free_space);

}


void delete_all_directories(const char* path) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        ESP_LOGE("FAT", "Failed to open directory: %s", path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char full_path[300];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    delete_all_directories(full_path); // Recursively delete subdirectories
                    rmdir(full_path); // Delete the directory
                    ESP_LOGI("FAT", "Deleted directory: %s", full_path);
                }
            } else {
                unlink(full_path); // Delete the file
                ESP_LOGI("FAT", "Deleted file: %s", full_path);
            }
        }
    }

    closedir(dir);
}

void format_fatfs() {
    ESP_LOGI("FAT", "Formatting FATFS partition...");
    // Erase the entire partition
    // esp_err_t ret = wl_erase_range(s_wl_handle, 0, WL_FLASH_SIZE);
    // if (ret != ESP_OK) {
    //     ESP_LOGE("FAT", "Failed to erase FATFS partition: %s", esp_err_to_name(ret));
    // } else {
    //     ESP_LOGI("FAT", "FATFS partition formatted successfully");

    // }

    // Start the format process
    if (formatfatfs() == ESP_OK) {
        ESP_LOGI(TAG, "Format complete, proceeding...");
        vTaskDelay(100);

        // Mount the filesystem after formatting
    if (init_fatfs()== ESP_OK) {
        // Create directories
        print_memory_status();
        create_directories();
    }

    } else {
        ESP_LOGE(TAG, "Format failed");
    }

}

 esp_err_t formatfatfs() {
      ESP_LOGI(TAG, "Formatting FATFS...");

    // Unmount the filesystem
    esp_err_t unmount_err = esp_vfs_fat_spiflash_unmount("/fatfs", NULL);
    if (unmount_err != ESP_OK) {
        ESP_LOGW(TAG, "Unmount failed, filesystem may not be mounted");
    }

    // Find the partition
    const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, PARTITION_LABEL);
    if (partition == NULL) {
        ESP_LOGE(TAG, "Failed to find partition with label: %s", PARTITION_LABEL);
        return ESP_ERR_NOT_FOUND;
    }

    // Erase the partition (format)
    ESP_LOGI(TAG, "Erasing partition: %s", partition->label);
    esp_err_t erase_err = esp_partition_erase_range(partition, 0, partition->size);
    if (erase_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase partition: %s", partition->label);
        return erase_err;
    }

    ESP_LOGI(TAG, "Partition erased successfully");

    return ESP_OK;
}


/*

void save_face_data(uint16_t person_id, const char* name, uint32_t image_width, uint32_t image_hight, const uint8_t* image_data ,const char* directory) {
   

    // ESP_LOGI("save_face_data", "person_id: %d name: %s image_width: %d image_hight: %d directory: %s",person_id,name,image_width,image_hight, directory);


    char file_name[64];

    snprintf(file_name, sizeof(file_name), "%s/%d.dat",directory, person_id);


    FILE* f = fopen(file_name, "wb");
    if (f == NULL) {
        // ESP_LOGE("save_face_data", "Failed to open file for writing");
        return;
    }

    // Write person ID
    fwrite(&person_id, sizeof(person_id), 1, f);
    
    // Write name length and name
    uint8_t name_len = strlen(name);
    fwrite(&name_len, sizeof(name_len), 1, f);
    fwrite(name, name_len, 1, f);

    // Write image dimensions
    fwrite(&image_width, sizeof(image_width), 1, f);
    fwrite(&image_hight, sizeof(image_hight), 1, f);

    // Calculate the image size in bytes for RGB565 format (2 bytes per pixel)
    uint32_t image_size = image_width * image_hight * 2;
    
    // Write the image data
    fwrite(image_data, image_size, 1, f);

    fclose(f);
    ESP_LOGI("save_face_data", "Face data saved to %s", file_name);
}



*/



// void save_face_data(uint16_t person_id, const char* name, uint32_t image_width, uint32_t image_height, const uint8_t* image_data) {


//     // ESP_LOGI("save_face_data", "person_id: %d name: %s image_width: %d image_hight: %d directory: %s",person_id,name,image_width,image_height, directory);


//     char file_name[64];

//     // Create the file path
//     snprintf(file_name, sizeof(file_name), "/fatfs/faces/%d.dat", person_id);


//     // // Check if the file already exists
//     // if (access(file_name, F_OK) == 0) {  // F_OK checks if the file exists
//     //     // If the file exists, delete it
//     //     if (remove(file_name) != 0) {
//     //         // Handle error deleting file (optional logging or error handling here)
//     //         // ESP_LOGE("save_face_data", "Failed to delete existing file: %s", file_name);
//     //         return;
//     //     }
//     //     // ESP_LOGI("save_face_data", "Existing file deleted: %s", file_name);
//     // }

//     // Open the file for writing (it will be created if it doesn't exist)
//     FILE* f = fopen(file_name, "wb");
//     if (f == NULL) {
//         // ESP_LOGE("save_face_data", "Failed to open file for writing");
//         return;
//     }

//     // Write person ID
//     fwrite(&person_id, sizeof(person_id), 1, f);

//     // Write name length and name
//     uint8_t name_len = strlen(name);
//     fwrite(&name_len, sizeof(name_len), 1, f);
//     fwrite(name, name_len, 1, f);

//     // Write image dimensions
//     fwrite(&image_width, sizeof(image_width), 1, f);
//     fwrite(&image_height, sizeof(image_height), 1, f);

//     // Calculate the image size in bytes for RGB565 format (2 bytes per pixel)
//     uint32_t image_size = image_width * image_height * 2;

//     // Write the image data
//     fwrite(image_data, image_size, 1, f);

//     fclose(f);
//     ESP_LOGI("save_face_data", "Face data saved to %s", file_name);
// }


void save_face_data(uint16_t person_id, const char* name, uint8_t image_width, uint8_t image_height, const uint8_t* image_data, const char* directory) {


    // ESP_LOGI("save_face_data", "person_id: %d name: %s image_width: %d image_hight: %d directory: %s",person_id,name,image_width,image_height, directory);


    char file_name[64];

    // Create the file path
    snprintf(file_name, sizeof(file_name), "%s/%d.dat", directory, person_id);

    // Check if the file already exists
    if (access(file_name, F_OK) == 0) {  // F_OK checks if the file exists
        // If the file exists, delete it
        if (remove(file_name) != 0) {
            // Handle error deleting file (optional logging or error handling here)
            // ESP_LOGE("save_face_data", "Failed to delete existing file: %s", file_name);
            return;
        }
        // ESP_LOGI("save_face_data", "Existing file deleted: %s", file_name);
    }

    // Open the file for writing (it will be created if it doesn't exist)
    FILE* f = fopen(file_name, "wb");
    if (f == NULL) {
        // ESP_LOGE("save_face_data", "Failed to open file for writing");
        return;
    }

    // Write person ID
    fwrite(&person_id, sizeof(person_id), 1, f);

    // Write name length and name
    uint8_t name_len = strlen(name);
    fwrite(&name_len, sizeof(name_len), 1, f);
    fwrite(name, name_len, 1, f);

    // Write image dimensions
    fwrite(&image_width, sizeof(image_width), 1, f);
    fwrite(&image_height, sizeof(image_height), 1, f);

    // Calculate the image size in bytes for RGB565 format (2 bytes per pixel)
    uint16_t image_size = image_width * image_height * 2;

    // Write the image data
    fwrite(image_data, image_size, 1, f);

    fclose(f);
    ESP_LOGI("save_face_data", "Face data saved to %s", file_name);
}

bool delete_face_data(uint16_t person_id , const char * directory) {
    
    char file_name[64];
    snprintf(file_name, sizeof(file_name), "%s/%d.dat",directory, person_id);

    int res = remove(file_name);
    if (res == 0) {
        // ESP_LOGI("delete_face_data", "Deleted face data for Person ID %d", person_id);
        return true;
    } else {
        // ESP_LOGE("delete_face_data", "Failed to delete face data for Person ID %d", person_id);
        return false;
    }
}



// for binary
void write_log_attendance(uint16_t person_id, uint8_t* timestamp) {
    char log_file[31];  // File path: /fatfs/log/2412121716.log
    snprintf(log_file, sizeof(log_file), "%s/%02d%02d%02d%02d%02d.log", ATTENDANCE_DIR, 
             timestamp[0], timestamp[1], timestamp[2], timestamp[3], timestamp[4]);

    ESP_LOGI("log_attendance", "file name: %s", log_file);

    FILE* f = fopen(log_file, "ab");  // Open in "append binary" mode
    if (f == NULL) {
        ESP_LOGE("log_attendance", "Failed to open log file for writing");
        return;
    }

    // Write timestamp (6 bytes) (1 byte) time formet and person ID (2 bytes) as binary data
    fwrite(timestamp, sizeof(uint8_t), 6, f);

    uint8_t temp= dspTimeFormet==1?0x0C:0x18;
    fwrite(&temp, sizeof(uint8_t), 1, f);// save time formet

    temp= person_id>>8;
    fwrite(&temp, sizeof(uint8_t), 1, f);  // Write person ID once (high bytes)
    temp= person_id & 0x00ff;
    fwrite(&temp, sizeof(uint8_t), 1, f);  // Write person ID once (low bytes)

    fclose(f);
    ESP_LOGI("attendance", "Attendance ID: %d logged in file: %s", person_id, log_file);
}



void process_attendance_files() {

    DIR *dir;
    struct dirent *entry;


    if ((dir = opendir(ATTENDANCE_DIR)) == NULL) {
        // ESP_LOGE("Attendance", "Failed to open directory: %s", ATTENDANCE_DIR);
        return;
    }
    while ((entry = readdir(dir)) != NULL) {

        dataAvailable = true;

        if (entry->d_type == DT_REG) {  // Only process regular files
            char file_path[30];
            memset(file_path,0,sizeof(file_path));
            strcat(file_path, ATTENDANCE_DIR);
            strcat(file_path, "/");
            strcat(file_path, entry->d_name);
            
            ESP_LOGI("log", "Procesing...%s", file_path);

            // Send the file via STOMP
            if (sendFilePath(file_path)) {
                // If successful, delete the file
                if (remove(file_path) == 0) {
                    ESP_LOGI("log", "deleted file: %s", file_path);
                    break;  // Stop after sending and deleting one file
                } else {
                    // ESP_LOGE("log", "Failed to delete file: %s", file_path);
                }
            } else {
                // ESP_LOGE("log", "Failed to send file via STOMP: %s", file_path);
            }
        }
    }
    closedir(dir);
}


bool sendFilePath(const char *filePath) {
    // Open file and read content here
    // Example: Read file into buffer
 FILE *file = fopen(filePath, "rb");  // Open the file in binary mode
    if (file == NULL) {
        ESP_LOGE("log", "Failed to open file: %s", filePath);
        return false;
    }

    // Move to the end of the file to get its size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);  // Move back to the beginning

    if (fileSize % 9 != 0) {  // Each log entry is 8 bytes
        ESP_LOGE("log", "Corrupted file: %s (fileSize not a multiple of 9)", filePath);
        fclose(file);
        return false;
    }

    // Allocate buffer to hold the entire file
    char *fileContent = (char *)heap_caps_malloc(fileSize, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (fileContent == NULL) {
        ESP_LOGE("log", "Failed to allocate memory for file content");
        fclose(file);
        return false;
    }

    // Read the entire file content into the buffer
    fread(fileContent, 1, fileSize, file);
    fclose(file);

    // ESP_LOGW("log", "data after read");

    // for(uint16_t i=0; i<fileSize;i++){

    //     printf("%d ",fileContent[i]);

    // }

    // Allocate buffer for the wss message
    // We'll start the message with 'L ' and add space after each 8-byte log entry
    size_t stompMessageSize = fileSize + (fileSize / 9) + 2;  // 8 bytes + space for each log entry, plus 'L '
    char *stompMessage = (char *)heap_caps_malloc(stompMessageSize, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (stompMessage == NULL) {
        ESP_LOGE("log", "Failed to allocate memory for STOMP message");
        heap_caps_free(fileContent);
        return false;
    }

    // Start the STOMP message with 'L '
    stompMessage[0] = 'L';
    size_t stompIdx = 1;

    // Loop through the file content and process each log entry (9 bytes)
    for (long i = 0; i < fileSize; i += 9) {

        // stompMessage[stompIdx] = ' ';
        // stompIdx++;

        // Copy the 9 bytes (6 bytes timestamp + 2 bytes person ID +1 byte time formet) from the log
        memcpy(&stompMessage[stompIdx], &fileContent[i], 9);
        stompIdx += 9;

        if(stompIdx+9<=stompMessageSize){
            stompMessage[stompIdx] = ' ';
            stompIdx ++;  
        }

        ESP_LOGW("log", "stomp idx %d  stompMessageSize %d",stompIdx ,stompMessageSize);

        // Add a space after each log entry

    }

    // Null-terminate the STOMP message
    stompMessage[stompIdx] = '\0';

    ESP_LOGW("log", "data after encode data len %d ",stompIdx);
    for(uint16_t i=0; i<stompIdx;i++){

        printf("%d ",stompMessage[i]);

    }

    // Send the STOMP message
    if (!sendToWss((uint8_t *)stompMessage, stompIdx)) {
        // ESP_LOGE("log", "Error sending log via wss");
        heap_caps_free(fileContent);
        heap_caps_free(stompMessage);
        return false;
    }


    // Free allocated buffers
    heap_caps_free(fileContent);
    heap_caps_free(stompMessage);

    return true;
}


bool process_and_send_faces(uint16_t id) {
    dataAvailable = true;

    // Create the file name based on the provided ID
    char file_name[30];
    snprintf(file_name, sizeof(file_name), "%s/%d.dat", FACE_DIRECTORY, id);

    // ESP_LOGW("process_and_send_faces", "Open file for reading: %s", file_name);

    // Open the file for reading
    FILE* f = fopen(file_name, "rb");
    if (f == NULL) {
        // ESP_LOGE("process_and_send_faces", "Failed to open file for reading: %s", file_name);
        return false;  // Correct return type for bool
    }

    // Read metadata
    uint16_t person_id;
    uint8_t name_len;
    char name[64];
    uint8_t image_width;
    uint8_t image_height;

    fread(&person_id, sizeof(person_id), 1, f);
    fread(&name_len, sizeof(name_len), 1, f);
    fread(name, name_len, 1, f);
    name[name_len] = '\0';  // Null-terminate the name

    fread(&image_width, sizeof(image_width), 1, f);
    fread(&image_height, sizeof(image_height), 1, f);

    // Calculate the image size and allocate memory
    const uint16_t image_length = (image_width * image_height) * 2;
    uint8_t* image_data = (uint8_t *)heap_caps_malloc(image_length, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (image_data == NULL) {
        // ESP_LOGE("process_and_send_faces", "Failed to allocate memory for image data");
        fclose(f);
        return false;
    }

    // Read the image data from the file
    fread(image_data, image_length, 1, f);
    fclose(f);  // Close the file after reading





    // Send the image data using the `imagesent` function
    if (!imagesent(image_data, image_length, image_height, image_width, name, person_id)) {
        // ESP_LOGE("process_and_send_faces", "Failed to send file: %s", file_name);
        heap_caps_free(image_data);  // Free the memory in case of failure
        return false;
    }

    // Free the allocated memory after successful send
    heap_caps_free(image_data);

    // Return true for success
    return true;
}


// bool syncFace(const camera_fb_t *src, imageData_t **person) {
    
//     DIR *dir;
//     struct dirent *entry;
//     bool syncAvailable = false;



//     if ((dir = opendir(SYNC_DIR)) == NULL) {
//         // ESP_LOGE("display_faces", "Failed to open directory: %s", FACE_DIRECTORY);
//         return syncAvailable;
//     }

//     uint16_t image_length = 0;

//     while ((entry = readdir(dir)) != NULL) {
//         if (entry->d_type == DT_REG) {
//             char file_name[64];
//             memset(file_name, 0, sizeof(file_name));
//             strcat(file_name, SYNC_DIR);
//             strcat(file_name, "/");
//             strcat(file_name, entry->d_name);

//             FILE* f = fopen(file_name, "rb");
//             if (f == NULL) {
//                 // ESP_LOGE("display_faces", "Failed to open file for reading: %s", file_name);
//                 continue;
//             }

//             uint16_t person_id;
//             uint8_t name_len;
//             char name[30];
//             uint32_t image_width;
//             uint32_t image_height;

//             // Read metadata
//             fread(&person_id, sizeof(person_id), 1, f);
//             fread(&name_len, sizeof(name_len), 1, f);
//             fread(name, name_len, 1, f);
//             name[name_len] = '\0';

//             fread(&image_width, sizeof(image_width), 1, f);
//             fread(&image_height, sizeof(image_height), 1, f);

//             image_length = image_width * image_height * 2;


//             // ESP_LOGI("save_face_data", "name: %s id %d image_width: %d image_hight: %d ",name,person_id,image_width,image_height);



//             // Allocate memory for image data
//             uint8_t* image_data = (uint8_t *)heap_caps_malloc(image_length, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
//             if (image_data == NULL) {
//                 // ESP_LOGE("display_faces", "Failed to allocate memory for image data, requesting %d bytes", image_length);

//                 fclose(f);
//                 continue;
//             }

//             fread(image_data, image_length, 1, f);
//             fclose(f);

//             // Allocate memory for person and populate the fields
//             *person = (imageData_t *)malloc(sizeof(imageData_t));
//             if (*person == NULL) {
//                 ESP_LOGE("display_faces", "Failed to allocate memory for person structure");
//                 // heap_caps_free(image_data);  // Free allocated memory for image data
//                 continue;
//             }

//             // Populate the person structure
//             (*person)->buf = image_data;
//             (*person)->len = image_length;
//             (*person)->width = image_width;
//             (*person)->height = image_height;
//             (*person)->id = (uint16_t)person_id;

//             // Allocate memory for the name and copy it
//             (*person)->Name = (char *)malloc(name_len + 1);  // +1 for the null terminator
//             if ((*person)->Name == NULL) {
//                 // ESP_LOGE("display_faces", "Failed to allocate memory for person name");
//                 heap_caps_free(image_data);
//                 continue;
//             }
//             strcpy((*person)->Name, name);

//             // Now insert the FATFS image into the middle of the camera buffer (src)
//             uint8_t *data = src->buf;           // Camera frame buffer
//             uint8_t *fatfs_data = (*person)->buf;  // FATFS image data

//             // image_width = (*person)->width;
//             // image_height = (*person)->height;
//             size_t offset_x = (src->width - image_width) / 2;   // Center the FATFS image horizontally
//             size_t offset_y = (src->height - image_height) / 2; // Center the FATFS image vertically

//             int8_t bytes_per_pixel = 2;  // Assuming RGB565 format, which uses 2 bytes per pixel

//             // Loop through the height and width of the FATFS image
//             for (uint16_t i = 0; i < image_height; i++) {
//                 for (uint16_t j = 0; j < image_width; j++) {
//                     // Calculate the position in the camera buffer to insert the pixel
//                     int camera_pixel_index = ((offset_y + i) * src->width + (offset_x + j)) * bytes_per_pixel;
//                     // Calculate the position in the FATFS image buffer
//                     int fatfs_pixel_index = (i * image_width + j) * bytes_per_pixel;

//                     // Copy the FATFS image pixel to the camera buffer (RGB565, 2 bytes per pixel)
//                     data[camera_pixel_index] = fatfs_data[fatfs_pixel_index];         // High byte of RGB565
//                     data[camera_pixel_index + 1] = fatfs_data[fatfs_pixel_index + 1]; // Low byte of RGB565
//                 }
//             }
//             syncAvailable = true;
//             // ESP_LOGI("display_faces", "Person ID: %d, Name: %s, Image inserted at (%d, %d)", (*person)->id, (*person)->Name, offset_x, offset_y);
//         }
//         break;  // Read only the first valid file
//     }

//     closedir(dir);
//     return syncAvailable;
// }

bool syncFace(const camera_fb_t *src, imageData_t **person) {
    
    DIR *dir;
    struct dirent *entry;
    bool syncAvailable = false;

    if ((dir = opendir(SYNC_DIR)) == NULL) {
        return syncAvailable;
    }

    uint16_t image_length = 0;

    // Allocate memory for person
    *person = (imageData_t *)malloc(sizeof(imageData_t));
    if (*person == NULL) {
        closedir(dir);
        return false;  // Return immediately if memory allocation fails
    }
    memset(*person, 0, sizeof(imageData_t));  // Initialize to avoid issues

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // Only consider regular files
            char file_name[64];
            memset(file_name, 0, sizeof(file_name));
            strcat(file_name, SYNC_DIR);
            strcat(file_name, "/");
            strcat(file_name, entry->d_name);

            FILE* f = fopen(file_name, "rb");
            if (f == NULL) {
                continue;  // Skip this file if it cannot be opened
            }

            uint16_t person_id;
            uint8_t name_len;
            char name[30];
            uint32_t image_width;
            uint32_t image_height;

            // Read metadata
            fread(&person_id, sizeof(person_id), 1, f);
            fread(&name_len, sizeof(name_len), 1, f);
            fread(name, name_len, 1, f);
            name[name_len] = '\0';

            fread(&image_width, sizeof(image_width), 1, f);
            fread(&image_height, sizeof(image_height), 1, f);

            image_length = image_width * image_height * 2;


            // ESP_LOGI("save_face_data", "name: %s id %d image_width: %d image_hight: %d ",name,person_id,image_width,image_height);

            // Ensure the image fits in the camera buffer
            if (image_width > src->width || image_height > src->height) {
                fclose(f);
                continue;  // Skip this image if it doesn't fit in the camera buffer
            }

            // Allocate memory for image data
            uint8_t* image_data = (uint8_t *)heap_caps_malloc(image_length, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
            if (image_data == NULL) {
                fclose(f);
                continue;
            }

            fread(image_data, image_length, 1, f);
            fclose(f);

            // Allocate memory for the name and copy it
            (*person)->Name = (char *)malloc(name_len + 1);  // +1 for the null terminator
            if ((*person)->Name == NULL) {
                heap_caps_free(image_data);
                continue;  // Handle memory allocation failure
            }
            strcpy((*person)->Name, name);

            // Populate the person structure
            (*person)->buf = image_data;
            (*person)->len = image_length;
            (*person)->width = image_width;
            (*person)->height = image_height;
            (*person)->id = person_id;

            // Insert FATFS image into the camera buffer (centered)
            uint8_t *data = src->buf;  // Camera frame buffer
            uint8_t *fatfs_data = (*person)->buf;  // FATFS image data
            size_t offset_x = (src->width - image_width) / 2;   // Center the FATFS image horizontally
            size_t offset_y = (src->height - image_height) / 2; // Center the FATFS image vertically

            int8_t bytes_per_pixel = 2;  // Assuming RGB565 format

            for (uint16_t i = 0; i < image_height; i++) {
                for (uint16_t j = 0; j < image_width; j++) {
                    int camera_pixel_index = ((offset_y + i) * src->width + (offset_x + j)) * bytes_per_pixel;
                    int fatfs_pixel_index = (i * image_width + j) * bytes_per_pixel;

                    // Copy the FATFS image pixel to the camera buffer
                    data[camera_pixel_index] = fatfs_data[fatfs_pixel_index];         // High byte
                    data[camera_pixel_index + 1] = fatfs_data[fatfs_pixel_index + 1]; // Low byte
                }
            }

            syncAvailable = true;
            break;  // Exit after processing the first valid file
        }
    }

    closedir(dir);

    // If no valid image was found, free the person structure
    if (!syncAvailable) {
        if ((*person)->buf != NULL) {
            heap_caps_free((*person)->buf);
        }
        if ((*person)->Name != NULL) {
            free((*person)->Name);
        }
        free(*person);
        *person = NULL;
    }

    return syncAvailable;
}


bool display_faces(camera_fb_t *buff) {


    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(FACE_DIRECTORY)) == NULL) {
        // ESP_LOGE("display_faces", "Failed to open directory: %s", FACE_DIRECTORY);
        return false; // Return false to indicate failure to open directory
    }
    uint16_t image_length=0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // Only process regular files
            char file_name[64]; // Increased buffer size to accommodate longer paths
            memset(file_name, 0, sizeof(file_name));
            strcat(file_name, FACE_DIRECTORY);
            strcat(file_name, "/");
            strcat(file_name, entry->d_name);

            // ESP_LOGI("display_faces", "Opening file for reading: %s", file_name);

            FILE* f = fopen(file_name, "rb");
            if (f == NULL) {
                ESP_LOGE("display_faces", "Failed to open file for reading: %s", file_name);
                continue; // Skip this file and continue with the next
            }

            uint16_t person_id;
            uint8_t name_len;
            char name[64];
            uint8_t image_width;
            uint8_t image_height;

            // Read metadata
            fread(&person_id, sizeof(person_id), 1, f);
            fread(&name_len, sizeof(name_len), 1, f);
            fread(name, name_len, 1, f);
            name[name_len] = '\0'; // Null-terminate the name string

            fread(&image_width, sizeof(image_width), 1, f);
            fread(&image_height, sizeof(image_height), 1, f);

            // Calculate image size in bytes for RGB565 format (2 bytes per pixel)
            image_length = image_width * image_height * 2;




            // Allocate memory for image data
            uint8_t* image_data = (uint8_t *)heap_caps_malloc(image_length, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
            if (image_data == NULL) {
                // ESP_LOGE("display_faces", "Failed to allocate memory for image data");
                fclose(f);
                continue;
            }

            // Read image data
            fread(image_data, image_length, 1, f);
            fclose(f);

            uint8_t imageXPoss = (320/2)-(image_width/2);

            scaleAndDisplayImageInFrame(image_data,  image_width, image_height, buff, imageXPoss, 39);

            // Free allocated memory for image data
            heap_caps_free(image_data);


        }
    }
    closedir(dir);
    return true; // Return true to indicate successful processing
}


bool pendingData() {
    DIR *dir;
    struct dirent *entry;
    bool dAvailable = false;

    // Check files in ATTENDANCE_DIR
    if ((dir = opendir(ATTENDANCE_DIR)) == NULL) {
        // ESP_LOGE("Attendance", "Failed to open directory: %s", ATTENDANCE_DIR);
        return false;
    }

    // Iterate through all entries in ATTENDANCE_DIR
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // Only process regular files
            // ESP_LOGI("Attendance", "File available: %s", entry->d_name);
            dAvailable = true;
        }
    }
    closedir(dir);

    if (!dAvailable) {
        // ESP_LOGI("Attendance", "No data available in %s", ATTENDANCE_DIR);
    }else return true;

    // Check files in FACE_DIRECTORY
    if ((dir = opendir(FACE_DIRECTORY)) == NULL) {
        // ESP_LOGE("Face", "Failed to open directory: %s", FACE_DIRECTORY);
        return false;
    }

    // Reset dataAvailable for face data check
    dAvailable = false;

    // Iterate through all entries in FACE_DIRECTORY
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // Only process regular files
            // ESP_LOGI("Face", "File available: %s", entry->d_name);
            dAvailable = true;
        }
    }
    closedir(dir);

    if (!dAvailable) {
        // ESP_LOGI("Face", "No data available in %s", FACE_DIRECTORY);
    }else return true;



    // Check files in ATTENDANCE_DIR
    if ((dir = opendir(SYNC_DIR)) == NULL) {
        ESP_LOGE("SYNC_DIR", "Failed to open directory: %s", SYNC_DIR);
        return false;
    }

    // Iterate through all entries in ATTENDANCE_DIR
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // Only process regular files
            ESP_LOGI("SYNC_DIR", "File available: %s", entry->d_name);
            dAvailable = true;
        }
    }
    closedir(dir);

    if (!dAvailable) {
        // ESP_LOGI("Attendance", "No data available in %s", ATTENDANCE_DIR);
    }else return true;

    return dAvailable;
}