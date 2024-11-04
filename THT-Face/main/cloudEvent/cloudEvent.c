

#include "cloudEvent.h"
#include "who_button.h"
#include "st7789.h"


#define     TAG_ENROL       "ENROL"


// #define IDLEENROL               0
// #define ENROLING                0x01
// #define ENROLED                 0x02
// #define DUPLICATE               0x03

// #define DELETE_CMD              0X04
// #define DELETED                 0X05
// #define ID_INVALID              0X06


extern volatile uint8_t CmdEvent;
extern char personName[20];
extern uint16_t personId;

extern key_state_t key_state;
extern TickType_t enrolTimeOut;


void process_command(const char* buffer ) {
    
    // if(strlen(buffer)>10)resizeBuffer();

    // printf("process_command :%s\n", buffer);
    CPUBgflag=1;


  // Check if the buffer starts with "cmdEnrol" (case-sensitive)
    if (strncmp(buffer, "cmdenrol", strlen("cmdenrol")) == 0) {
       
    const char* ptr = buffer;  // Start pointer

    // Skip the command type "cmdenrol" by moving the pointer forward
    const char cmd[] = "cmdenrol";
    size_t cmd_length = strlen(cmd);
    if (memcmp(ptr, cmd, cmd_length) != 0) {
        // printf("Invalid command.\n");
        return;
    }
    ptr += cmd_length;

    // Skip any spaces between "cmdenrol" and the name
    ptr++;

    // Now extract the name
    char Name[25];

    size_t name_length = 0;
    while (*ptr != ' ') {  // Stop when space or null is found
        // if (name_length < sizeof(Name) - 1) {  // Prevent buffer overflow
            Name[name_length++] = *ptr;
        // }
        ptr++;
    }
    Name[name_length] = '\0';  // Null-terminate the name
    // Skip spaces after the name
    ptr++;
    // Extract the 2-byte CRC
    uint16_t rxCrc = (ptr[0] << 8) | ptr[1];  // Read the next two bytes as CRC

    // printf("Received Name: %s\n", Name);
    // printf("Received CRC: %x\n", rxCrc);
    uint16_t calculated_crc = crc16(Name, strlen(Name));

    printf("CRC  CALCULATED: %x\n", calculated_crc);

    if (calculated_crc == rxCrc) {

        CmdEvent = ENROLING_EVENT;

        enrolTimeOut = xTaskGetTickCount();

        // printf("CRC check passed.\n");
        // printf("Received Name: %s\n", Name);
        memset(personName,0,sizeof(personName));
        memcpy(personName,Name,strlen(Name));
        sleepTimeOut = xTaskGetTickCount();
        sleepEnable=WAKEUP;
        key_state=KEY_SHORT_PRESS;

        return;

    } else {

        // printf("CRC check failed.\n");
        // memset(tcpBuffer, 0, strlen(tcpBuffer));
        CmdEvent = NAME_DATA_ERROR;
        return;
    }
    }else if(strncmp(buffer, "uploadimage", strlen("uploadimage")) == 0){

        // CPUBgflag=1;
        sleepTimeOut = xTaskGetTickCount();// imediate wake if display in sleep mode
        sleepEnable=WAKEUP;
        uint16_t tempid = buffer[12]<<8|buffer[13];
        printf("giveimage: %d\n",tempid);
        process_and_send_faces(tempid);
        // CPUBgflag=0;



    }else if(strncmp(buffer, "imagedl", strlen("imagedl")) == 0){


        uint16_t tempid = buffer[8]<<8|buffer[9];
        // printf("giveimage: %d\n",tempid);
        if(delete_face_data(tempid,FACE_DIRECTORY)){

            CmdEvent = IMAGE_DELETE_SUC;
        }else CmdEvent = IMAGE_DELETE_FAIL;

    }else if(strncmp(buffer, "cmddl", strlen("cmddl")) == 0){

  
        const char* ptr = buffer;  // Start pointer
        // Skip the command type "cmdenrol" by moving the pointer forward
        const char cmd[] = "cmddl";
        size_t cmd_length = strlen(cmd);
        if (memcmp(ptr, cmd, cmd_length) != 0) {
            // printf("Invalid command.\n");
            return;
        }
        ptr += cmd_length;

         // Skip any spaces between "cmdenrol" and the name
        ptr++;

        // Now extract the name
        char Name[20];
        size_t name_length = 0;
        while (*ptr != ' ') {  // Stop when space or null is found
            if (name_length < sizeof(Name) - 1) {  // Prevent buffer overflow
                Name[name_length++] = *ptr;
            }
            ptr++;
        }
        Name[name_length] = '\0';  // Null-terminate the name
        // printf("Received Name: %s\n", Name);
        // Skip spaces after the name
        ptr++;
        uint16_t tempId = (ptr[0] << 8) | ptr[1];  // Read the next two bytes as id
        // printf("Received id: %x\n", tempId);

        uint16_t rxCrc = (ptr[3] << 8) | ptr[4];  // Read the next two bytes as CRC
        char crcPac[25]; // assuming max length of ID + name is 25

        memset(crcPac, 0, sizeof(crcPac));
        memcpy(&crcPac,&tempId,sizeof(tempId));
        strncpy(crcPac+strlen(crcPac), Name, strlen(Name));

        const uint16_t calculated_crc = crc16(crcPac, strlen(crcPac));
        // printf("CRC RCV: %x\n", rxCrc);
        printf("CRC16 CALCULATED: %x\n", calculated_crc);

        if (calculated_crc == rxCrc) {

            key_state = KEY_DOUBLE_CLICK;
            strncpy(personName, Name, strlen(Name));
            personName[strlen(Name)] = '\0'; // Null terminate the name string
            personId = tempId; // for test delete person by their ID

            return;

        } else {
            CmdEvent = ID_DATA_ERROR;
        }

    }else if (strncmp(buffer, "syncperson", strlen("syncperson")) == 0) {

        bool dataOk= true;
        const char* ptr = buffer;
        // Skip the command "syncperson"
        const char cmd[] = "syncperson";
        size_t cmd_length = strlen(cmd);
        if (memcmp(ptr, cmd, cmd_length) != 0) {
            // printf("Invalid command.\n");
            return;
        }
        ptr += cmd_length;

        // Allocate memory for the sync structure
        imageData_t* syncperson = (imageData_t*)heap_caps_malloc(sizeof(imageData_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        if (syncperson == NULL) {
            // printf("Memory allocation failed for sync\n");
            return;
        }

        // Extract the name
        ptr++;  // Skip spaces after the command

        uint16_t NameCrc = (ptr[0] << 8) | ptr[1];  // Read the next two bytes as CRC


        ptr += 3;  // Move pointer 



        size_t name_length = 0;
        while (*ptr != ' ') {  // Stop when space is found
            syncperson->Name[name_length++] = *ptr++;

        }
        syncperson->Name[name_length] = '\0';  // Null-terminate the name
        const uint16_t calculated_NameCrc = crc16(syncperson->Name, name_length);
        // Skip space after the name
         ptr++;

        syncperson->id = (ptr[0] << 8) | ptr[1];  // Read the next two bytes as id


        ptr += 3;  // Move pointer past height and width

        // Extract height and width
        syncperson->height = ptr[0];
        syncperson->width = ptr[1];

        ptr += 3;  // Move pointer past height and width

        uint16_t imageCrc = (ptr[0] << 8) | ptr[1];  // Read the next two bytes as CRC

        ptr += 3;  // Move pointer past height and width

        // Calculate the buffer size (height * width * 2) and allocate memory
        size_t buffer_size = (syncperson->height * syncperson->width) * 2;

        // ESP_LOGI("save_face_data", "name: %s id %d image_width: %d image_hight: %d",syncperson->Name,syncperson->id ,syncperson->width,syncperson->height);
        
        syncperson->buf = (uint8_t*)heap_caps_malloc(buffer_size,  MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        if (syncperson->buf == NULL) {
            printf("Memory allocation failed for buffer\n");
            return;
        }
        // Copy the buffer data from the input
        memcpy(syncperson->buf, ptr, buffer_size);
        // printf("buff size get %d, pointer size %d\n", buffer_size, (int)sizeof(syncperson->buf));

        const uint16_t calculated_imageCrc = crc16((char*)syncperson->buf, buffer_size);

        ESP_LOGI("save_face_data", "NAME CRC RX %x CALCULATE %x name: %s id %d image_width: %d image_hight: %d image crc %x  Calculate imag crc %x",\
        NameCrc,calculated_NameCrc ,\
        syncperson->Name,syncperson->id ,syncperson->width,syncperson->height,\
        imageCrc,calculated_imageCrc );

        // if(calculated_imageCrc!=imageCrc || calculated_NameCrc!=NameCrc){

        //     // printf("CRC mismatch: invlid data.\n");
        //     CmdEvent = SYNC_DATA_ERROR;
        //     dataOk = false;     
        // }

        // if(memcmp(syncperson->buf, ptr, buffer_size) == 1 ) {

        //     // printf("Data mismatch: received and saved data are different.\n");
        //     CmdEvent = SYNC_SAVED_FAIL;
        //     dataOk = false;     
        // } 

        for(uint16_t i=0; i< buffer_size;i++){

            printf("%x ",syncperson->buf[i]);

        }
        // CPUBgflag=1;

        // Call the save function to store the data
        if(dataOk)save_face_data(syncperson->id, syncperson->Name, syncperson->width, syncperson->height, syncperson->buf, SYNC_DIR);
        // Properly free the allocated memory for buffer and syncperson
        // CPUBgflag=0;

        ESP_LOGE("sync", "sync image saved ");
        vTaskDelay(100);
        if (syncperson->buf != NULL) {
            heap_caps_free(syncperson->buf);
        }
        
        CmdEvent = SYNC_SAVED;



    }
    else if(strncmp(buffer, "cmdsync", strlen("cmdsync")) == 0){

        enrolTimeOut = xTaskGetTickCount();
        key_state=KEY_SYNC;
        ESP_LOGE("cmdsync ", "start cmd sync");

        

    }else if(strncmp(buffer, "cmddump", strlen("cmddump")) == 0){

        // char dump[30]; // assuming 32-bit uint can be represented in 11 chars + null terminator

        // snprintf(personIdStr, sizeof(personIdStr), "AED %s %u",personName,personId);


        key_state = KEY_DUMP;


        // if (!stompSend("ADI",PUBLISH_TOPIC)) {
        //     ESP_LOGE("ID DELETE", "Error sending ACK");
        // } else {
        //     ESP_LOGI(TAG_ENROL, "back to idle mode\n");
        // }


    }else if(strncmp(buffer, "restart", strlen("restart")) == 0){


        if (!sendToWss((uint8_t*)"ADRESTART",10)) {
            // ESP_LOGE("ID DELETE", "Error sending ACK");
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
        }
        vTaskDelay(200);
        esp_restart();

    }else if (strncmp(buffer, "clean", strlen("clean")) == 0){

        CPUBgflag=1;
        format_fatfs();

        if (!sendToWss((uint8_t*)"ACFATFS",8)) {
            // ESP_LOGE("ID DELETE", "Error sending ACK");
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
        }
        vTaskDelay(100);
        CPUBgflag=0;
        vTaskDelay(200);
        esp_restart();


    }else if(strncmp(buffer, "time", strlen("time"))==0){

        ESP_LOGI(TAG_ENROL, "time formet %d %d %d %d %d %d %d", buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10] , buffer[11]);

        time_library_time_t initial_time = {buffer[5], buffer[6], buffer[7] ,buffer[8], buffer[9], buffer[10] ,buffer[11]+1};//     year, month, day,weekday, hour, minute, second;
        time_library_set_time(&initial_time, 1);
        CmdEvent = TIME_UPDATE;


    }else if(strncmp(buffer,"htime", strlen("htime"))==0){

        // ESP_LOGI(TAG_ENROL, "time formet %d", buffer[6]);

        dspTimeFormet =  buffer[6]==0x0C ? 1 : 0 ;// assign time formet 
        CmdEvent = TIME_FORMET_UPDATE;
        save_time_format(dspTimeFormet);

    }
    CPUBgflag=0;

}

void eventFeedback(void){


    // ESP_LOGE("CmdEvent start", "%x",CmdEvent);
    switch (CmdEvent)
    {
    case DELETED:
        // ack for delete id
        // if (!stompSend("ADI",PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)"ADI",4)) {

            // ESP_LOGE("ID DELETE", "Error sending ACK");
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode ID DELETE\n");
            CmdEvent = IDLE_EVENT;
        }        
        break;
    case ID_INVALID:
        // nack for delete invalide id
        // if (!stompSend("NDII",PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)"NDII",5)) {
            // ESP_LOGE("ID DELETE", "Error sending NACK");
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }
        break;    
    case ID_DATA_ERROR:

        // nack for ID DATA ERROR
        // if (!stompSend("NIDE",PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)"NIDE",5)) {

            // ESP_LOGE("ID DELETE", "Error sending NACK");
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;    
    case NAME_DATA_ERROR:

        // nack for NAME DATA ERROR
        // if (!stompSend("NNDE",PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)"NNDE",5)) {
            // ESP_LOGE("ID DELETE", "Error sending NACK");
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;    
    case ENROLED:{

        char personIdStr[30]; // assuming 32-bit uint can be represented in 11 chars + null terminator


        snprintf(personIdStr, sizeof(personIdStr), "AED %s %u",personName,personId);

        // if (!stompSend(personIdStr,PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)personIdStr,strlen(personIdStr))) {
            //  ESP_LOGE("feedback", "Error sending id: errno %d", errno);
        } else {
            // ESP_LOGI(TAG_ENROL, "id sent to client\n");
            CmdEvent = IDLE_EVENT;
        }

        break; 
    }   


    case SYNC_SAVED:


        if (!sendToWss((uint8_t*)"ASSPI",6)) {
            //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }



        break;

    case SYNC_SAVED_FAIL:


        if (!sendToWss((uint8_t*)"NSSPI",6)) {
            //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;


    case SYNC_DATA_ERROR:

        if (!sendToWss((uint8_t*)"NSDE",5)) {
            //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;

    case DUPLICATE:

        // nack for duplicate person
        if (!sendToWss((uint8_t*)"NDP",4)) {
            //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }


        break; 


    case  SYNC_DONE:{


        char personIdStr[30]; // assuming 32-bit uint can be represented in 11 chars + null terminator

        snprintf(personIdStr, sizeof(personIdStr), "ASD %s %u",personName,personId);
        if (!sendToWss((uint8_t*)personIdStr,strlen(personIdStr))) {
            //  ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            // ESP_LOGI(TAG_ENROL, "id sent to client\n");
            CmdEvent = IDLE_EVENT;
            memset(personName,0,sizeof(personName));
            personId=0;
        }
        break; 

    }
    case  SYNC_DUPLICATE:

        // nack for sync duplicate person
        if (!sendToWss((uint8_t*)"NSDP",5)) {
            //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }



        break;  
    case SYNC_ERROR:

        // nack for sync error
        if (!sendToWss((uint8_t*)"NSER",5)) {
            //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;

    case ENROLMENT_TIMEOUT:

        // nack for time out
        // if (!stompSend("NETO",PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)"NETO",5)) {

            //ESP_LOGE(TAG_ENROL, "Error sending id: errno %d", errno);
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;

        }
        break;  

    case TIME_UPDATE:

        if (!sendToWss((uint8_t*)"ATU",4)) {// ack for time update

            //ESP_LOGE(TAG_ENROL, "Error sending id: errno %d", errno);
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;

        }

        break; 


    case TIME_FORMET_UPDATE:

        if (!sendToWss((uint8_t*)"ATFU",5)) {

            //ESP_LOGE(TAG_ENROL, "Error sending id: errno %d", errno);
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;
    case IMAGE_DELETE_SUC:

        if (!sendToWss((uint8_t*)"ADPI",5)) {

            //ESP_LOGE(TAG_ENROL, "Error sending id: errno %d", errno);
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }
       break;

    case IMAGE_DELETE_FAIL:

            if (!sendToWss((uint8_t*)"NDPII",6)) {

            //ESP_LOGE(TAG_ENROL, "Error sending id: errno %d", errno);
        } else {
            // ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;
    
    default:
        break;
    }
}


// uint32_t crc32(const char *buf, size_t len) {
//     uint32_t crc = 0xFFFFFFFF;
//     for (size_t i = 0; i < len; i++) {
//         uint8_t byte = buf[i];
//         crc = crc_table[(crc ^ byte) & 0xFF] ^ (crc >> 8);
//     }
//     return crc ^ 0xFFFFFFFF;
// }


// CRC-16-CCITT table for faster computation
uint16_t crc16(const char *buf, size_t len) {


    uint16_t crc = 0x0000; // Initialize with 0x0000
    for (size_t i = 0; i < len; i++) {
        uint8_t byte = buf[i];
        crc = (crc >> 8) ^ crc16_table[(crc & 0xFF) ^ byte];
    }
    return crc;
}

