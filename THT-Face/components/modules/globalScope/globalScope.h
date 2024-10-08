#pragma once
//----------------Debug tag name------------------------------------
// #define     TAG             "Wss"
// #define     TAGSTOMP        "STOMP_CLIENT"


//------------------------------------------------------------------

#define DEVICE_VERSION_ID "AA00"// device v
#define WL_FLASH_SIZE      2097152//FATFS size

// CMD Enrol handle from server--------------------------------------
#define IDLE_EVENT          0x00
#define ENROLING_EVENT      0x01
#define ENROLED             0x02
#define DUPLICATE           0x03
#define DELETE_CMD          0X04
#define DELETED             0X05
#define ID_INVALID          0X06
#define ID_DATA_ERROR       0X07
#define NAME_DATA_ERROR     0X08
#define ENROLMENT_TIMEOUT   0x09

#define SYNC_DUPLICATE   0x0A
#define SYNC_DONE   0x0B
#define SYNC_ERROR   0x0C


// sleep ----------------
#define SLEEP   0x01
#define WAKEUP  0x00
//---------------------


//--------------------------------------------------------------------

//-------------------Time value in ms---------------------------------
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
#define TIMEOUT_10000_MS     1000
#define TIMEOUT_12000_MS      1200
#define TIMEOUT_20000_MS      2000
#define TIMEOUT_15_S          1500
#define TIMEOUT_30_S          3000
#define TIMEOUT_45_S          4500
#define TIMEOUT_1_MIN         6000
#define TIMEOUT_2_MIN         12000
#define TIMEOUT_5_MIN         30000
//-------------------------------------------------------------------


//-------------------topic-------------------------------------------

// #define     PUBLISH_TOPIC         "/app/cloud"
// #define     SUBCRIBE_TOPIC        "/topic/cloud"


#define     PUBLISH_TOPIC         "/app/messages"
#define     SUBCRIBE_TOPIC        "/topic/AA00242829068"
//-------------------------------------------------------------------

//-----------network status code-------------------------------------
#define     WIFI_DISS           0x00
#define     WIFI_CONNECTED      0x01
#define     WSS_CONNECTED       0x02
#define     STOMP_CONNECTED     0x03
//-------------------------------------------------------------------

//-------------------------------------------------------------
#define     IMAGE_CHANK_SIZE      900 //512//760//256 //128
#define     CHANK_SIZE            1000 //512//760//256 //128
#define     MAXTRY                30

//-----------------WSS addr------------------------------------------------------------------
// #define     THT             "wss://grozziieget.zjweiting.com:3091/CloudSocket-Dev/websocket/"

#define     THT             "wss://grozziieget.zjweiting.com:3091/WebSocket-Binary/ws"

#define     HOST            "grozziieget.zjweiting.com" 
#define     PORT            3091
#define     PATH            "/CloudSocket-Dev/websocket/"
//-------------------------------------------------------------------------------------------

//--------------flash addr for store big array-----------------------------------------------
#define DATA_FLASH __attribute__((section(".rodata")))
//-------------------------------------------------------------------------------------------





#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
