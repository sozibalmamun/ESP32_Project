#pragma once


#define DEVICE_VERSION_ID "AA00"
#define WL_FLASH_SIZE      2097152//2097152 // 2 MB in bytes 190464  

// CMD Enrol handle from server
#define IDLEENROL       0x00
#define ENROLING        0x01
#define ENROLED         0x02
#define DUPLICATE       0x03

#define DELETE_CMD      0X04
#define DELETED         0X05
#define ID_INVALID      0X06




#define     PUBLISH_TOPIC       "/app/cloud"
#define     SUBCRIBE_TOPIC       "/topic/cloud"
#define     CHANK_SIZE      256 //512//760//256 //128



#define     THT             "wss://grozziieget.zjweiting.com:3091/CloudSocket-Dev/websocket/"
#define     HOST            "grozziieget.zjweiting.com"
#define     PORT            3091
#define     PATH            "/CloudSocket-Dev/websocket/"


#ifdef __cplusplus
extern "C" {
#endif





#ifdef __cplusplus
}
#endif
