

#include "enrol.h"

#define TIMEOUT_50_MS         5
#define TIMEOUT_100_MS        10
#define TIMEOUT_120_MS        12
#define TIMEOUT_150_MS        15
#define TIMEOUT_200_MS        20
#define TIMEOUT_300_MS        30
#define TIMEOUT_500_MS        50
#define TIMEOUT_1000_MS       100
#define TIMEOUT_2000_MS       200
#define TIMEOUT_3000_MS       300
#define TIMEOUT_4000_MS       400
#define TIMEOUT_5000_MS       500
#define TIMEOUT_6000_MS       600
#define TIMEOUT_7000_MS       700
#define TIMEOUT_9000_MS       900
#define TIMEOUT_10000_MS      1000
#define TIMEOUT_12000_MS      1200
#define TIMEOUT_20000_MS      2000
#define TIMEOUT_15_S          1500
#define TIMEOUT_30_S          3000
#define TIMEOUT_45_S          4500
#define TIMEOUT_1_MIN         6000
#define TIMEOUT_2_MIN         12000
#define TIMEOUT_5_MIN         30000

#define     TAG_ENROL       "ENROL"


#define IDLEENROL               0
#define ENROLING                0x01
#define ENROLED                 0x02
#define DUPLICATE               0x03

#define DELETE_CMD              0X04
#define DELETED                 0X05
#define ID_INVALID              0X06


extern volatile uint8_t CmdEnroll;
extern char personName[20];
extern uint16_t personId;

// volatile uint8_t  CmdEnroll=IDLEENROL;
// char personName[20];
// uint16_t personId;
extern bool stompSend(char * buff, char* topic);



TickType_t erolTimeOut;


void process_command(const char* buffer) {
    
    // if(strlen(buffer)>10)resizeBuffer();
  // Check if the buffer starts with "cmdEnrol" (case-sensitive)
    if (strncmp(buffer, "cmdEnrol", strlen("cmdEnrol")) == 0) {
       
        // init_crc16_table();
        // Extract the name (assuming space separates name and ID)
        const char* name_start = buffer + strlen("cmdEnrol") + 1;
        const char* space_pos = strchr(name_start, ' ');
        if (space_pos == NULL) {
            // Handle invalid format (no space)
            return;
        }
        strncpy(personName, name_start, space_pos - name_start);
        personName[space_pos - name_start] = '\0'; // Null terminate the name string

        // Extract the 4-character hex CRC
        char crc_str[5];
        strncpy(crc_str, space_pos + 1, 4);
        crc_str[4] = '\0'; // Null terminate the CRC string
        // Convert the hex string to a 16-bit integer
        uint16_t rxCrc = hex_to_uint16(crc_str);

        // Check for end command string (case-sensitive)
        const char* end_cmd_pos = strstr(buffer, "cmdEnd");
        if (end_cmd_pos != NULL) {
            // Data reception complete, print information
            printf("Enrollment data received:\n");
            printf("  - CRC RCV: %x\n",rxCrc);

            uint16_t calculated_crc = crc16(personName, strlen(personName));
            printf("  - CRC16 CALCULATED: %x\n", calculated_crc);

            if (calculated_crc == rxCrc) {
                printf("\ncmd enroll flag status %d",CmdEnroll);

                CmdEnroll = ENROLING;
                erolTimeOut = xTaskGetTickCount();
                printf("CRC check passed.\n");
                printf("  - Name: %s\n", personName);
                memset(tcpBuffer, 0, strlen(tcpBuffer));

                enrolOngoing();

                return;
            } else {
                printf("CRC check failed.\n");
                memset(tcpBuffer, 0, strlen(tcpBuffer));
                CmdEnroll = IDLEENROL;
                return;
            }
        }

    }else if(strncmp(buffer, "cmddl", strlen("cmddl")) == 0){

        // Extract the name (assuming space separates name and ID)
        const char* name_start = buffer + strlen("cmddl") + 1;
        const char* space_pos = strchr(name_start, ' ');
        if (space_pos == NULL) {
            // Handle invalid format (no space)

            return;
        }
        char id[2];//65535 ffff  

        strncpy(id, name_start, space_pos - name_start);
        // id[space_pos - name_start] = '\0'; // Null terminate the name string
        printf("id  char: %s\n",id);

        uint16_t tempid= chartou16(id);
        // uint16_t tempid= 1;

        printf("id  hex: %x\n",tempid);


        // Extract the 2-character CRC
        char crc_str[2];
        strncpy(crc_str, space_pos + 1, 2);

        // Check for end command string (case-sensitive)
        const char* end_cmd_pos = strstr(buffer, "cmdend");
        if (end_cmd_pos != NULL) {
            // Data reception complete, print information
            uint16_t rxCrc = chartou16(crc_str);
            printf("  - CRC RCV: %x\n",rxCrc);

            uint16_t calculated_crc = getCRC16(tempid);
            printf("  - CRC16 CALCULATED: %x\n", calculated_crc);

            // personId= tempid;
            personId= 1;// for test delete person by there id




            if (calculated_crc == rxCrc) {

                // CmdEnroll = DELETE_CMD;
                // printf("CRC check passed.\n");
                // printf("  - Name: %s\n", id);
                // idDeletingOngoing();

                return;
            } else {
                printf("CRC check failed.\n");
                
                CmdEnroll = DELETE_CMD;
                idDeletingOngoing();

                return;
            }

        }
    }
}


void enrolOngoing(void){

        bool cmd=true;
        while(cmd){

            if(CmdEnroll==ENROLED){

                char personIdStr[12]; // assuming 32-bit uint can be represented in 11 chars + null terminator
                snprintf(personIdStr, sizeof(personIdStr), "%u", personId);
                if (!stompSend(personIdStr,PUBLISH_TOPIC)) {
                    //  ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
                } else {
                    ESP_LOGI(TAG_ENROL, "id sent to client\n");
                    CmdEnroll = IDLEENROL;
                    cmd=false;
                }
            }else if(CmdEnroll==DUPLICATE){

                ESP_LOGI(TAG_ENROL, "duplicate ack\n");

                // nack for duplicate person
                if (!stompSend("NDP",PUBLISH_TOPIC)) {
                    //ESP_LOGE(TAGSOCKET, "Error sending id: errno %d", errno);
                } else {
                    ESP_LOGI(TAG_ENROL, "back to idle mode\n");
                    CmdEnroll = IDLEENROL;
                    cmd=false;
                }
            }else {

                TickType_t TimeOut = xTaskGetTickCount();
        
                if (TimeOut-erolTimeOut> TIMEOUT_15_S ){
                // ESP_LOGI(TAG_ENROL, "not acking\n");
                // send(client_sock, "\nwait for..", 8, 0);
                CmdEnroll = IDLEENROL;

                // nack for time out
                if (!stompSend("NETO",PUBLISH_TOPIC)) {
                    //ESP_LOGE(TAG_ENROL, "Error sending id: errno %d", errno);
                } else {
                    ESP_LOGI(TAG_ENROL, "back to idle mode\n");
                    cmd=false;
                }
                printf("\ncmd enroll flag status %d",CmdEnroll);
                // vTaskDelay(10);

                }
            }

        }
}

void idDeletingOngoing(void){
    bool cmd=true;
    while(cmd){



        if(CmdEnroll==DELETED){

            // ack for delete id
            if (!stompSend("ADI",PUBLISH_TOPIC)) {
                ESP_LOGE("ID DELETE", "Error sending ACK");
            } else {
                ESP_LOGI(TAG_ENROL, "back to idle mode\n");
                CmdEnroll = IDLEENROL;
                cmd=false;
            }

        }else if(CmdEnroll==ID_INVALID){

            // nack for delete invalide id
            if (!stompSend("NDII",PUBLISH_TOPIC)) {
                ESP_LOGE("ID DELETE", "Error sending NACK");
            } else {
                ESP_LOGI(TAG_ENROL, "back to idle mode\n");
                CmdEnroll = IDLEENROL;
                cmd=false;
            }

        }else {

            // ESP_LOGI(TAG_ENROL, "wait for idle mode\n");

        }
    }

}
