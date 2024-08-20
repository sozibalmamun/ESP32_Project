
#include "fs.h"

static const char *TAG = "FAT";

// Function to initialize and mount FAT filesystem
esp_err_t init_fatfs(void) {
    esp_err_t ret;
    const esp_vfs_fat_mount_config_t mount_config = {
        .max_files = 5,
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
    // Ensure that FATFS is mounted before creating directories
    // if (init_fatfs() != ESP_OK) {
    //     ESP_LOGE("create_directories", "FATFS not mounted. Cannot create directories.");
    //     return;
    // }

    struct stat st;
    if (stat(BASE_PATH "/log", &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            ESP_LOGI("FAT", "Directory /attendance already exists");
        } else {
            ESP_LOGE("FAT", "/attendance exists but is not a directory");
        }
    } else {
        // If the directory does not exist, try to create it
        int res = mkdir(BASE_PATH "/log", 0777);
        if (res != 0 && errno != EEXIST) {
            ESP_LOGE("FAT", "Failed to create directory: %s", BASE_PATH "/attendance");
        } else {
            ESP_LOGI("FAT", "Directory /attendance created");
        }
    }


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
        } else {
            ESP_LOGI("FAT", "Directory /faces created");
        }
    }




    // // Create /faces directory
    // int res = mkdir(BASE_PATH "/faces", 0777);
    // if (res != 0 && errno != EEXIST) {
    //     ESP_LOGE("FAT", "Failed to create directory: %s", BASE_PATH "/faces");
    // } else {
    //     ESP_LOGI("FAT", "Directory /faces created");
    // }

    // // Create /attendance directory
    // res = mkdir(BASE_PATH "/attendance", 0777);
    // if (res != 0 && errno != EEXIST) {
    //     ESP_LOGE("FAT", "Failed to create directory: %s", BASE_PATH "/attendance");
    // } else {
    //     ESP_LOGI("FAT", "Directory /attendance created");
    // }
}

// void init_fatfs() {

//     esp_vfs_fat_mount_config_t mount_config = {
//         .format_if_mount_failed = true,//false int 
//         .max_files = 5,
//         .allocation_unit_size = CONFIG_WL_SECTOR_SIZE,
//     };

//     esp_err_t ret = esp_vfs_fat_spiflash_mount("/storage", "storage", &mount_config, &s_wl_handle);
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(ret));
//         return;
//     }

//     ESP_LOGI(TAG, "FATFS mounted successfully");

// }

void print_memory_status(void) {

    FATFS *fs = s_wl_handle;  // Use the mounted FATFS object
    DWORD free_clusters;
    FRESULT res;

    // Get the free space in clusters
    res = f_getfree("/storage", &free_clusters, &fs);
    if (res != FR_OK) {
        ESP_LOGE(TAG, "Failed to get free space: %d", res);
        return;
    }

    // Calculate total and free space in bytes
    DWORD total_clusters = fs->n_fatent - 2;  // Total clusters - reserved clusters
    DWORD cluster_size = fs->csize * 512;     // Size of one cluster in bytes
    DWORD total_space = total_clusters * cluster_size;
    DWORD free_space = free_clusters * cluster_size;
    DWORD used_space = total_space - free_space;

    ESP_LOGI(TAG, "Total Space: %u  bytes", total_space);
    ESP_LOGI(TAG, "Free Space:  %u  bytes", free_space);
    ESP_LOGI(TAG, "Used Space:  %u  bytes", used_space);
}




// void create_directories() {
//     // Create /faces directory
// //     int res = mkdir(FACE_DIRECTORIES, 0777);
// //     if (res != 0 && errno != EEXIST) {

// //         ESP_LOGE("create_directories", "Failed to create /faces directory");
// //     }

// //     // Create /attendance directory
// //     res = mkdir(LOG_DIRECTORIES, 0777);
// //     if (res != 0 && errno != EEXIST) {

// //         ESP_LOGE("create_directories", "Failed to create /attendance directory");

// //     }

// // Check if the directory creation is successful
// if (mkdir("/storage/faces", 0777) != 0) {
//     ESP_LOGE(TAG, "Failed to create directory: /storage/faces");
// }

// if (mkdir("/storage/attendance", 0777) != 0) {
//     ESP_LOGE(TAG, "Failed to create directory: /storage/attendance");
// }



// }



void save_face_data(uint32_t person_id, const char* name, uint32_t image_width, uint32_t image_length, const uint8_t* image_data) {
    char file_name[64];
    snprintf(file_name, sizeof(file_name), "/fatfs/faces/person_%d.dat", person_id);

    FILE* f = fopen(file_name, "wb");
    if (f == NULL) {
        ESP_LOGE("save_face_data", "Failed to open file for writing");
        return;
    }

    // Write person ID, name, image dimensions, and image data as in previous examples
    fwrite(&person_id, sizeof(person_id), 1, f);
    uint32_t name_len = strlen(name);
    fwrite(&name_len, sizeof(name_len), 1, f);
    fwrite(name, name_len, 1, f);
    fwrite(&image_width, sizeof(image_width), 1, f);
    fwrite(&image_length, sizeof(image_length), 1, f);
    fwrite(image_data, image_length, 1, f);

    fclose(f);
    ESP_LOGI("save_face_data", "Face data saved to %s", file_name);
}

void read_face_data(uint32_t person_id) {
    char file_name[64];
    snprintf(file_name, sizeof(file_name), "/fatfs/faces/person_%d.dat", person_id);

    FILE* f = fopen(file_name, "rb");
    if (f == NULL) {
        ESP_LOGE("read_face_data", "Failed to open file for reading");
        return;
    }

    uint32_t read_person_id;
    uint32_t name_len;
    char name[64];
    uint32_t image_width;
    uint32_t image_length;

    // Read person ID
    fread(&read_person_id, sizeof(read_person_id), 1, f);

    // Read name length and name
    fread(&name_len, sizeof(name_len), 1, f);
    fread(name, name_len, 1, f);
    name[name_len] = '\0'; // Null-terminate the string

    // Read image width
    fread(&image_width, sizeof(image_width), 1, f);

    // Read image length
    fread(&image_length, sizeof(image_length), 1, f);

    // Read image data
    uint8_t* image_data = malloc(image_length);
    if (image_data == NULL) {
        ESP_LOGE("read_face_data", "Failed to allocate memory for image data");
        fclose(f);
        return;
    }
    fread(image_data, image_length, 1, f);

    fclose(f);

    // Do something with the data (e.g., display it or process it)
    ESP_LOGI("read_face_data", "Read Data - ID: %d, Name: %s, Width: %d, Length: %d", read_person_id, name, image_width, image_length);

    // Free the allocated memory for image data
    free(image_data);
}


void delete_face_data(uint32_t person_id) {
    char file_name[64];
    snprintf(file_name, sizeof(file_name), "/fatfs/faces/person_%d.dat", person_id);

    int res = remove(file_name);
    if (res == 0) {
        ESP_LOGI("delete_face_data", "Deleted face data for Person ID %d", person_id);
    } else {
        ESP_LOGE("delete_face_data", "Failed to delete face data for Person ID %d", person_id);
    }
}

void wright_log_attendance(uint32_t person_id, const char* timestamp) {
    char log_file_name[64];
    snprintf(log_file_name, sizeof(log_file_name), "/fatfs/log/%s.log", timestamp);//storage


    FILE* f = fopen(log_file_name, "a");
    if (f == NULL) {
        ESP_LOGE("log_attendance", "Failed to open log file for writing");
        return;
    }

    // Write attendance log: person ID and timestamp
    fprintf(f, "%d %s\n", person_id, timestamp);

    fclose(f);
    ESP_LOGI("log_attendance", "Attendance logged for Person ID %d at %s", person_id, timestamp);
}
void read_attendance_log(const char* date) {
    char log_file_name[64];
    snprintf(log_file_name, sizeof(log_file_name), "/fatfs/log/%s.log", date);

    FILE* f = fopen(log_file_name, "r");
    if (f == NULL) {
        ESP_LOGE("read_attendance_log", "Failed to open attendance log for date %s", date);
        return;
    }

    char line[128];
    while (fgets(line, sizeof(line), f) != NULL) {
        // ESP_LOGI("read_attendance_log", "%s", line);
    }
    ESP_LOGI("read_attendance_log", "%s", line);

    fclose(f);
    // ESP_LOGI("read_attendance_log", "Finished reading attendance log for date %s", date);
}
void delete_attendance_log(const char* date) {
    char log_file_name[64];
    snprintf(log_file_name, sizeof(log_file_name), "/fatfs/log/%s.log", date);

    int res = remove(log_file_name);
    if (res == 0) {
        ESP_LOGI("delete_attendance_log", "Deleted attendance log for date %s", date);
    } else {
        ESP_LOGE("delete_attendance_log", "Failed to delete attendance log for date %s", date);
    }
}

