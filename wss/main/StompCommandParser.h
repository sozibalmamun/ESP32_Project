#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "Stomp.h"

#ifdef __cplusplus
extern "C"
{
#endif


namespace Stomp {


class StompCommandParser {

  public:

StompCommand parse(char *data) {
    StompCommand cmd;
    memset(&cmd, 0, sizeof(cmd)); // Initialize the command structure
    
    const char *EOL = "\n";
    const char *EOL2 = "\n\n";
    
    // Find headers start and body start
    char *headersStart = strstr(data, EOL);
    char *bodyStart = strstr(data, EOL2);
    
    if (headersStart == NULL) {
        cmd.command = strdup(data); // Assuming command is at the beginning if no headers
    } else {
        int headersLength = headersStart - data;
        cmd.command = (char *)malloc(headersLength + 1);
        strncpy(cmd.command, data, headersLength);
        cmd.command[headersLength] = '\0';
        headersStart += strlen(EOL);
    }
    
    if (bodyStart == NULL) {
        if (headersStart != NULL) {
            cmd.body = strdup(headersStart); // If no body, then the rest is headers
        } else {
            cmd.body = strdup(""); // No body found
        }
    } else {
        int headersLength = bodyStart - headersStart;
        cmd.body = (char *)malloc(headersLength + 1);
        strncpy(cmd.body, headersStart, headersLength);
        cmd.body[headersLength] = '\0';
        bodyStart += strlen(EOL2);
    }
    
    // Parse headers
    char *header = strtok(cmd.body, EOL);
    while (header != NULL) {
        // Split into key and value
        char *colon = strchr(header, ':');
        if (colon != NULL) {
            StompHeader h;
            int keyLength = colon - header;
            h.key = (char *)malloc(keyLength + 1);
            strncpy(h.key, header, keyLength);
            h.key[keyLength] = '\0';
            
            int valueLength = strlen(header) - keyLength - 1;
            h.value = (char *)malloc(valueLength + 1);
            strncpy(h.value, colon + 1, valueLength);
            h.value[valueLength] = '\0';
            
            cmd.headers[cmd.headers++] = h;
        }
        header = strtok(NULL, EOL);
    }
    
    return cmd;
}
};

}
#ifdef __cplusplus
}
#endif


