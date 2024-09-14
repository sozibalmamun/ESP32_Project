

#include "enrol.h"
#include "who_button.h"

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
  // Check if the buffer starts with "cmdEnrol" (case-sensitive)
    if (strncmp(buffer, "cmdenrol", strlen("cmdenrol")) == 0) {
       
        // init_crc16_table();
        // Extract the name (assuming space separates name and ID)
        const char* name_start = buffer + strlen("cmdenrol") + 1;
        const char* space_pos = strchr(name_start, ' ');
        if (space_pos == NULL) {
            // Handle invalid format (no space)
            return;
        }

        memset(personName, 0, sizeof(personName));
        strncpy(personName, name_start, space_pos - name_start);
        personName[space_pos - name_start] = '\0'; // Null terminate the name string

        // Extract the 4-character hex CRC
        char crc_str[5];
        strncpy(crc_str, space_pos + 1, 4);
        crc_str[4] = '\0'; // Null terminate the CRC string
        // Convert the hex string to a 16-bit integer
        uint16_t rxCrc = hex_to_uint16(crc_str);

        // Check for end command string (case-sensitive)
        const char* end_cmd_pos = strstr(buffer, "cmdend");
        if (end_cmd_pos != NULL) {
            // Data reception complete, print information
            // printf("  - CRC RCV: %x\n",rxCrc);
            uint16_t calculated_crc = crc16(personName, strlen(personName));
            printf("  - CRC16 CALCULATED: %x\n", calculated_crc);

            if (calculated_crc == rxCrc) {

                CmdEvent = ENROLING_EVENT;

                enrolTimeOut = xTaskGetTickCount();
                // printf("CRC check passed.\n");
                // printf("  - Name: %s\n", personName);
                // memset(tcpBuffer, 0, strlen(tcpBuffer));

                key_state=KEY_SHORT_PRESS;

                return;

            } else {

                // printf("CRC check failed.\n");
                // memset(tcpBuffer, 0, strlen(tcpBuffer));
                CmdEvent = NAME_DATA_ERROR;
                return;
            }
        }

    }else if(strncmp(buffer, "cmddl", strlen("cmddl")) == 0){

  
 // Extract the ID (assuming space separates ID and name)
        const char* id_start = buffer + strlen("cmddl") + 1;
        const char* space_pos1 = strchr(id_start, ' ');
        if (space_pos1 == NULL) {
            // Handle invalid format (no space)
            return;
        }

        char id[5];
        memset(id, 0, sizeof(id));
        strncpy(id, id_start, space_pos1 - id_start);
        id[space_pos1 - id_start] = '\0'; // Null terminate the ID string

        // Extract the name (assuming space separates name and CRC)

        const char* name_start = space_pos1 + 1;
        const char* space_pos2 = strchr(name_start, ' ');
        if (space_pos2 == NULL) {
            // Handle invalid format (no space)
            return;
        }

        // char personName[20]; // assuming max name length is 20
        memset(personName, 0, sizeof(personName));
        // strncpy(personName, name_start, space_pos2 - name_start);
        // personName[space_pos2 - name_start] = '\0'; // Null terminate the name string


        // Extract the 2-character CRC
        const char* crc_start = space_pos2 + 1;
        char crc_str[5]; // 2 characters + null terminator
        memset(crc_str, 0, sizeof(crc_str));
        strncpy(crc_str, crc_start, 4);
        crc_str[4] = '\0'; // Null terminate the CRC string


        // Check for end command string (case-sensitive)
        const char* end_cmd_pos = strstr(buffer, "cmdend");
        if (end_cmd_pos != NULL) {


            char id_and_name[25]; // assuming max length of ID + name is 25
            memset(id_and_name, 0, sizeof(id_and_name));
            strncpy(id_and_name, id, strlen(id));
            strcat(id_and_name, " ");
            strncpy(id_and_name + strlen(id_and_name), name_start, space_pos2 - name_start);

            const uint16_t calculated_crc = crc16(id_and_name, strlen(id_and_name));
            const uint16_t rxCrc = hex_to_uint16(crc_str);


            // printf("  - CRC RCV: %x\n", rxCrc);
            printf("  - CRC16 CALCULATED: %x\n", calculated_crc);

            if (calculated_crc == rxCrc) {

                key_state = KEY_DOUBLE_CLICK;
                strncpy(personName, name_start, space_pos2 - name_start);
                personName[space_pos2 - name_start] = '\0'; // Null terminate the name string
                personId = chartoDeci(id); // for test delete person by their ID

                // printf("\nid: %d name: %s",personId,personName);
                return;

            } else {
                CmdEvent = ID_DATA_ERROR;
            }

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


    }else if(strncmp(buffer, "restart", strlen("restart")) == 0){


        if (!stompSend("ADRESTART",PUBLISH_TOPIC)) {
            ESP_LOGE("ID DELETE", "Error sending ACK");
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
        }
        vTaskDelay(100);
        esp_restart();

    }else if (strncmp(buffer, "clean", strlen("clean")) == 0){

        CPUBgflag=1;
        format_fatfs();

        if (!stompSend("ACFATFS",PUBLISH_TOPIC)) {
            ESP_LOGE("ID DELETE", "Error sending ACK");
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
        }
        vTaskDelay(100);
        CPUBgflag=0;


    }else if(strncmp(buffer, "time", strlen("time"))==0){

    // time_library_time_t initial_time = {2024, 12, 12, 17, 16, 15};//     year, month, day, hour, minute, second;
    // time_library_init(&initial_time);

    }
}

void eventFeedback(void){


    // ESP_LOGE("CmdEvent start", "%x",CmdEvent);
    switch (CmdEvent)
    {
    case DELETED:
        // ack for delete id
        if (!stompSend("ADI",PUBLISH_TOPIC)) {
            ESP_LOGE("ID DELETE", "Error sending ACK");
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }        
        break;
    case ID_INVALID:
        // nack for delete invalide id
        if (!stompSend("NDII",PUBLISH_TOPIC)) {
            ESP_LOGE("ID DELETE", "Error sending NACK");
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }
        break;    
    case ID_DATA_ERROR:

        // nack for ID DATA ERROR
        if (!stompSend("NIDE",PUBLISH_TOPIC)) {
            ESP_LOGE("ID DELETE", "Error sending NACK");
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;    
    case NAME_DATA_ERROR:

        // nack for NAME DATA ERROR
        if (!stompSend("NNDE",PUBLISH_TOPIC)) {
            ESP_LOGE("ID DELETE", "Error sending NACK");
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;    
    case ENROLED:{

        char personIdStr[30]; // assuming 32-bit uint can be represented in 11 chars + null terminator

        snprintf(personIdStr, sizeof(personIdStr), "AED %s %u",personName,personId);
        if (!stompSend(personIdStr,PUBLISH_TOPIC)) {
            //  ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "id sent to client\n");
            CmdEvent = IDLE_EVENT;
        }

        break; 
    }   
    case DUPLICATE:

        // nack for duplicate person
        if (!stompSend("NDP",PUBLISH_TOPIC)) {
            //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }


        break; 


    case  SYNC_DONE:{


        char personIdStr[30]; // assuming 32-bit uint can be represented in 11 chars + null terminator

        snprintf(personIdStr, sizeof(personIdStr), "ASD %s %u",personName,personId);
        if (!stompSend(personIdStr,PUBLISH_TOPIC)) {
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
        if (!stompSend("NSDP",PUBLISH_TOPIC)) {
            //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }



        break;  
    case SYNC_ERROR:

        // nack for sync error
        if (!stompSend("NSER",PUBLISH_TOPIC)) {
            //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;
        }

        break;

    case ENROLMENT_TIMEOUT:

        // nack for time out
        if (!stompSend("NETO",PUBLISH_TOPIC)) {
            //ESP_LOGE(TAG_ENROL, "Error sending id: errno %d", errno);
        } else {
            ESP_LOGI(TAG_ENROL, "back to idle mode\n");
            CmdEvent = IDLE_EVENT;

        }
        break;  

    // case DUMP_REQ:

    //     break; 


    // case DELETED:
    //     /* code */
    //     break;
    
    default:
        break;
    }
}
