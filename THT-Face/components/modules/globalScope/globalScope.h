#pragma once


#define DEVICE_VERSION_ID "AA00"
#define WL_FLASH_SIZE      2097152//2097152 // 2 MB in bytes 190464  

// CMD Enrol handle from server
#define IDLE_EVENT          0x00
#define ENROLING_EVENT      0x01
#define ENROLED             0x02
#define DUPLICATE           0x03

#define DELETE_CMD          0X04
#define DELETED             0X05
#define ID_INVALID          0X06
#define ID_DATA_ERROR       0X07
#define NAME_DATA_ERROR     0X08

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




#define     PUBLISH_TOPIC         "/app/cloud"
#define     SUBCRIBE_TOPIC        "/topic/cloud"

#define     IMAGE_CHANK_SIZE      100 //512//760//256 //128
#define     CHANK_SIZE            512 //512//760//256 //128





#define     THT             "wss://grozziieget.zjweiting.com:3091/CloudSocket-Dev/websocket/"
#define     HOST            "grozziieget.zjweiting.com"
#define     PORT            3091
#define     PATH            "/CloudSocket-Dev/websocket/"



#define DATA_FLASH __attribute__((section(".rodata")))


#ifdef __cplusplus
extern "C" {
#endif





#ifdef __cplusplus
}
#endif
