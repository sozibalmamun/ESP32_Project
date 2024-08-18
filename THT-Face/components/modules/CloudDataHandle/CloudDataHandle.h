#pragma once

#include "stdbool.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif


bool imagesend( uint8_t* buff);
bool stompSend(char * buff, char* topic);


#ifdef __cplusplus
}
#endif