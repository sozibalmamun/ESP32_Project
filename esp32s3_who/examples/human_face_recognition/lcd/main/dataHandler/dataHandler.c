


#include "dataHandler.h"

extern void process_command(const char* buffer);
#define     DATA_HANDEL     "DATA HANDEL"


// void resizeBuffer() {
//     char startMarker[] = "cmdEnrol";
//     char endMarker[] = "cmdEnd";
    
//     char *start = strstr(tcpBuffer, startMarker);
//     char *end = strstr(tcpBuffer, endMarker);
    
//     if (start && end && end > start) {
//         end += strlen(endMarker); // Move pointer to end of endMarker
        
//         size_t newSize = end - start;
//         char newBuffer[newSize + 1];
//         strncpy(newBuffer, start, newSize);
//         newBuffer[newSize] = '\0'; // Null terminate the new buffer
        
//         printf("New Buffer: %s\n", newBuffer);
        
//         // Optionally, if you want to update the global tcpBuffer with resized data
//         memset(tcpBuffer, 0, sizeof(tcpBuffer));
//         strncpy(tcpBuffer, newBuffer, newSize);
//     } else {
//         printf("Markers not found or in wrong order\n");

//     }
// }

// CRC-32 table for faster computation
// void init_crc32_table() {
//     uint32_t polynomial = 0xEDB88320;
//     for (uint32_t i = 0; i < 256; i++) {
//         uint32_t c = i;
//         for (int j = 0; j < 8; j++) {
//             if (c & 1) {
//                 c = polynomial ^ (c >> 1);
//             } else {
//                 c = c >> 1;
//             }
//         }
//         crc_table[i] = c;
//     }
// }

// uint32_t crc32(const char *buf, size_t len) {
//     uint32_t crc = 0xFFFFFFFF;
//     for (size_t i = 0; i < len; i++) {
//         uint8_t byte = buf[i];
//         crc = crc_table[(crc ^ byte) & 0xFF] ^ (crc >> 8);
//     }
//     return crc ^ 0xFFFFFFFF;
// }


// CRC-16-CCITT table for faster computation

// void init_crc16_table(void) {
//     uint16_t polynomial = 0x1021;
//     for (uint16_t i = 0; i < 256; i++) {
//         uint16_t c = i << 8;
//         for (int j = 0; j < 8; j++) {
//             if (c & 0x8000) {
//                 c = (c << 1) ^ polynomial;
//             } else {
//                 c = c << 1;
//             }
//         }
//         crc16_table[i] = c;
//     }
// }

uint16_t crc16(const char *buf, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        uint8_t byte = buf[i];
        crc = crc16_table[((crc >> 8) ^ byte) & 0xFF] ^ (crc << 8);
    }
    return crc;
}



uint16_t getCRC16(uint16_t value) {
  uint16_t crc = 0x0000;
  uint8_t data[4];

  toArray(value, &data[0]);

  for (int index = 0 ; index < 4; ++index) {
    crc = crc16_table[(crc ^ data[index]) & 0x0f] ^ (crc >> 4);
    crc = crc16_table[(crc ^ (data[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }  
  return crc;
}




uint16_t hex_to_uint16(const char* hex_str) {
    uint16_t result=0x0000;
    sscanf(hex_str, "%4hx", &result);
    return result;
}

//////////////////////////////////////////////////////////////////////
uint32_t toint4(uint8_t *data_buffer) {
  uint32_t slotL = data_buffer[0];
  slotL = (slotL << 8) + data_buffer[1];
  slotL = (slotL << 8) + data_buffer[2];
  slotL = (slotL << 8) + data_buffer[3];

  return slotL;
}

///////////////////////////// To Array 4 /////////////////////////////
//////////////////////////////////////////////////////////////////////
void toArray4(uint32_t slotL, uint8_t *data_buffer) {
  data_buffer[3] = slotL & 0xFF;
  data_buffer[2] = (slotL >> 8)  & 0xFF;
  data_buffer[1] = (slotL >> 16) & 0xFF;
  data_buffer[0] = (slotL >> 24) & 0xFF;
}
//////////////////////////// To Array 2 //////////////////////////////
//////////////////////////////////////////////////////////////////////
void toArray(uint16_t slotL, uint8_t *data_buffer) {
  data_buffer[1] = slotL & 0xFF;
  data_buffer[0] = (slotL >> 8) & 0xFF;
}
uint16_t toint2(uint8_t *data_buffer){

  uint16_t intout = 0;
  intout = data_buffer[1] | (data_buffer[1] << 8);
  return intout;

}




void u16tochar (uint16_t data, char* buff) {
  buff[0] = data & 0xFF; 
  buff[1] = (data >> 8) & 0xFF;
}
void u32tochar (uint32_t data, char* buff) {
  //  char buff[4];
  uint8_t index = 0 ;
  buff[index] = data & 0xFF; index++;
  buff[index] = (data >> 8) & 0xFF; index++;
  buff[index] = (data >> 16) & 0xFF; index++;
  buff[index] = (data >> 24) & 0xFF; index++;
  //  return buff;
}
uint16_t chartou16 (char* data) {
  uint16_t intout = 0;

  uint16_t temp = data[1];
  intout = data[0] | (temp << 8);
  return intout;
}

uint32_t chartou32 (char* data) {
  uint32_t intout = 0;

  uint32_t temp = data[1];
  intout = data[0] | (temp << 8);
  temp = data[2];
  intout = intout | (temp << 16);
  temp = data[3];
  intout = intout | (temp << 24);
  return intout;
}



void dataHandele(const char *rx_buffer) {

    uint16_t len = strlen(rx_buffer);
    memset(tcpBuffer,0 ,len);
    // rx_buffer[len] = '\0';
    extractMessage(rx_buffer, tcpBuffer);

    // memcpy(&tcpBuffer,&rx_buffer ,len);

    // ESP_LOGI(DATA_HANDEL, "Received STOMP len:%d msg: \n%s", len , rx_buffer);

    ESP_LOGI(DATA_HANDEL, "extracted data \n%s",tcpBuffer);
    // Process received data here
    // if(strlen(tcpBuffer)>5)resizeBuffer();

    // if (strstr(tcpBuffer, "cmd") && strstr(tcpBuffer, "End") && strstr(tcpBuffer, "End") > strstr(tcpBuffer, "cmd")) {// varifi the cmd pattern

    // // if (strstr(tcpBuffer, "cmd") != NULL) {

    //     process_command(tcpBuffer);


    // }else{ 

    //     ESP_LOGE(TAG_ENROL, "invalid %d", errno);
    //     // memcpy(&tcpBuffer[tcpLen],&rx_buffer ,len);


    //     }

    process_command(tcpBuffer);


}

void extractMessage(const char *buffer, char *output) {
    // Locate the start of the JSON payload


// MESSAGE\ndestination:/topic/cloud\ncontent-type:text/plain;charset=UTF-8\nsubscription:sub-0\nmessage-id:513446-45973\ncontent-length:19\n\ncmddl Í crc cmdend\u0000"]


    //length:40\n\n{\"message\":\"cmdEnrol sozib 8dfb cmdEnd\"}\u0000";
    const char *jsonStart = strstr(buffer, "message\\\":\\\"");
    if (jsonStart) {
        jsonStart += strlen("message\\\":\\\""); // Move past the starting point

        // Locate the end of the message within the JSON payload
        const char *jsonEnd = strstr(jsonStart, "\\\"}");
        if (jsonEnd) {
            // Copy the message content into the output buffer
            size_t length = jsonEnd - jsonStart;
            strncpy(output, jsonStart, length);
            output[length] = '\0'; // Null-terminate the output string
        }
    }

}



// void resizeBuffer() {
//     char startMarker[] = "cmdEnrol";
//     char endMarker[] = "cmdEnd";
    
//     char *start = strstr(tcpBuffer, startMarker);
//     char *end = strstr(tcpBuffer, endMarker);
    
//     if (start && end && end > start) {
//         end += strlen(endMarker); // Move pointer to end of endMarker
        
//         size_t newSize = end - start;
//         char newBuffer[newSize + 1];
//         strncpy(newBuffer, start, newSize);
//         newBuffer[newSize] = '\0'; // Null terminate the new buffer
        
//         printf("New Buffer: %s\n", newBuffer);
        
//         // Optionally, if you want to update the global tcpBuffer with resized data
//         memset(tcpBuffer, 0, sizeof(tcpBuffer));
//         strncpy(tcpBuffer, newBuffer, newSize);
//     } else {
//         printf("Markers not found or in wrong order\n");

//     }
// }