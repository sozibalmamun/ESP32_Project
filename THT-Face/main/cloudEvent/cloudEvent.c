

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


void process_command(const char* buffer) {
    
    // if(strlen(buffer)>10)resizeBuffer();

    printf("process_command :%s\n", buffer);


  // Check if the buffer starts with "cmdEnrol" (case-sensitive)
    if (strncmp(buffer, "cmdenrol", strlen("cmdenrol")) == 0) {
       
    const char* ptr = buffer;  // Start pointer

    // Skip the command type "cmdenrol" by moving the pointer forward
    const char cmd[] = "cmdenrol";
    size_t cmd_length = strlen(cmd);
    if (memcmp(ptr, cmd, cmd_length) != 0) {
        printf("Invalid command.\n");
        return;
    }
    ptr += cmd_length;

    // Skip any spaces between "cmdenrol" and the name
    ptr++;

    // Now extract the name
    char personName[25];
    size_t name_length = 0;
    while (*ptr != ' ') {  // Stop when space or null is found
        if (name_length < sizeof(personName) - 1) {  // Prevent buffer overflow
            personName[name_length++] = *ptr;
        }
        ptr++;
    }
    personName[name_length] = '\0';  // Null-terminate the name
    // Skip spaces after the name
    ptr++;
    // Extract the 2-byte CRC
    uint16_t rxCrc = (ptr[0] << 8) | ptr[1];  // Read the next two bytes as CRC

    // printf("Received Name: %s\n", personName);
    // printf("Received CRC: %x\n", rxCrc);
    uint16_t calculated_crc = crc16(personName, strlen(personName));

    // printf("CRC high CALCULATED: %x\n", calculated_crc);

    if (calculated_crc == rxCrc) {

        CmdEvent = ENROLING_EVENT;

        enrolTimeOut = xTaskGetTickCount();

        // printf("CRC check passed.\n");
        // printf("  - Name: %s\n", personName);
        // memset(tcpBuffer, 0, strlen(tcpBuffer));
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
    }else if(strncmp(buffer, "giveimage", strlen("giveimage")) == 0){


        uint16_t tempid = buffer[10]<<8|buffer[11];
        // printf("giveimage: %d\n",tempid);
        process_and_send_faces(tempid);


    }else if(strncmp(buffer, "imagedl", strlen("imagedl")) == 0){


        uint16_t tempid = buffer[8]<<8|buffer[9];
        printf("giveimage: %d\n",tempid);
        if(delete_face_data(tempid)){

            CmdEvent = IMAGE_DELETE_SUC;
        }else CmdEvent = IMAGE_DELETE_FAIL;

    }else if(strncmp(buffer, "cmddl", strlen("cmddl")) == 0){

  
        const char* ptr = buffer;  // Start pointer
        // Skip the command type "cmdenrol" by moving the pointer forward
        const char cmd[] = "cmddl";
        size_t cmd_length = strlen(cmd);
        if (memcmp(ptr, cmd, cmd_length) != 0) {
            printf("Invalid command.\n");
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
        printf("CRC RCV: %x\n", rxCrc);
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

    }else if(strncmp(buffer, "cmdsync", strlen("cmdsync")) == 0){

        enrolTimeOut = xTaskGetTickCount();
        key_state=KEY_SYNC;

    }else if(strncmp(buffer, "cmddump", strlen("cmddump")) == 0){

        // char dump[30]; // assuming 32-bit uint can be represented in 11 chars + null terminator

        // snprintf(personIdStr, sizeof(personIdStr), "AED %s %u",personName,personId);


        key_state = KEY_DUMP;


        // if (!stompSend("ADI",PUBLISH_TOPIC)) {
        //     ESP_LOGE("ID DELETE", "Error sending ACK");
        // } else {
        //     ESP_LOGI(TAG_ENROL, "back to idle mode\n");
        // }

// esp_err_t lcd_st7789_rst(uint8_t cmd){

//     return LCD_WRITE_DATA(0x01);

// }

// esp_err_t lcd_st7789_sleep(uint8_t cmd);{

//     return LCD_WRITE_DATA(0x10);

// }



    }else if(strncmp(buffer, "restart", strlen("restart")) == 0){


        if (!sendToWss((uint8_t*)"ADRESTART",10)) {
            ESP_LOGE("ID DELETE", "Error sending ACK");
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
        }
        vTaskDelay(200);
        esp_restart();

    }else if (strncmp(buffer, "clean", strlen("clean")) == 0){

        CPUBgflag=1;
        format_fatfs();

        if (!sendToWss((uint8_t*)"ACFATFS",8)) {
            ESP_LOGE("ID DELETE", "Error sending ACK");
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
        }
        vTaskDelay(100);
        CPUBgflag=0;


    }else if(strncmp(buffer, "time", strlen("time"))==0){

        // ESP_LOGI(TAG_ENROL, "time formet %d %d %d %d %d %d", buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10]);

        time_library_time_t initial_time = {buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10]};//     year, month, day, hour, minute, second;
        time_library_init(&initial_time);
        CmdEvent = TIME_UPDATE;



    }else if(strncmp(buffer,"htime", strlen("htime"))==0){

        // ESP_LOGI(TAG_ENROL, "time formet %d", buffer[6]);

        dspTimeFormet =  buffer[6]==0x0C ? 1 : 0 ;// assign time formet 
        CmdEvent = TIME_FORMET_UPDATE;

    }
}

void eventFeedback(void){


    // ESP_LOGE("CmdEvent start", "%x",CmdEvent);
    switch (CmdEvent)
    {
    case DELETED:
        // ack for delete id
        // if (!stompSend("ADI",PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)"ADI",4)) {

            ESP_LOGE("ID DELETE", "Error sending ACK");
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }        
        break;
    case ID_INVALID:
        // nack for delete invalide id
        // if (!stompSend("NDII",PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)"NDII",5)) {
            ESP_LOGE("ID DELETE", "Error sending NACK");
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }
        break;    
    case ID_DATA_ERROR:

        // nack for ID DATA ERROR
        // if (!stompSend("NIDE",PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)"NIDE",5)) {

            ESP_LOGE("ID DELETE", "Error sending NACK");
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;    
    case NAME_DATA_ERROR:

        // nack for NAME DATA ERROR
        // if (!stompSend("NNDE",PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)"NNDE",5)) {
            ESP_LOGE("ID DELETE", "Error sending NACK");
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;    
    case ENROLED:{

        char personIdStr[30]; // assuming 32-bit uint can be represented in 11 chars + null terminator

        snprintf(personIdStr, sizeof(personIdStr), "AED %s %u",personName,personId);
        // if (!stompSend(personIdStr,PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)personIdStr,strlen(personIdStr))) {
            //  ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "id sent to client\n");
            CmdEvent = IDLE_EVENT;
        }

        break; 
    }   
    case DUPLICATE:

        // nack for duplicate person
        // if (!stompSend("NDP",PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)"NDP",4)) {
            //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }


        break; 


    case  SYNC_DONE:{


        char personIdStr[30]; // assuming 32-bit uint can be represented in 11 chars + null terminator

        snprintf(personIdStr, sizeof(personIdStr), "ASD %s %u",personName,personId);
        // if (!stompSend(personIdStr,PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)personIdStr,strlen(personIdStr))) {
            //  ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "id sent to client\n");
            CmdEvent = IDLE_EVENT;
            memset(personName,0,sizeof(personName));
            personId=0;
        }
        break; 

    }
    case  SYNC_DUPLICATE:

        // nack for sync duplicate person
        // if (!stompSend("NSDP",PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)"NSDP",5)) {
            //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }



        break;  
    case SYNC_ERROR:

        // nack for sync error
        // if (!stompSend("NSER",PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)"NSER",5)) {
            //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;

    case ENROLMENT_TIMEOUT:

        // nack for time out
        // if (!stompSend("NETO",PUBLISH_TOPIC)) {
        if (!sendToWss((uint8_t*)"NETO",5)) {

            //ESP_LOGE(TAG_ENROL, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;

        }
        break;  

    case TIME_UPDATE:

        if (!sendToWss((uint8_t*)"ATU",4)) {// ack for time update

            //ESP_LOGE(TAG_ENROL, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;

        }

        break; 


    case TIME_FORMET_UPDATE:

        if (!sendToWss((uint8_t*)"ATFU",5)) {

            //ESP_LOGE(TAG_ENROL, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;
    case IMAGE_DELETE_SUC:

        if (!sendToWss((uint8_t*)"ADPI",5)) {

            //ESP_LOGE(TAG_ENROL, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }
       break;

    case IMAGE_DELETE_FAIL:

            if (!sendToWss((uint8_t*)"NDPII",6)) {

            //ESP_LOGE(TAG_ENROL, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;
    
    default:
        break;
    }
}
