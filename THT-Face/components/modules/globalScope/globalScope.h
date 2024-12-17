#pragma once
//----------------Debug tag name------------------------------------
// #define     TAG             "Wss"
// #define     TAGSTOMP        "STOMP_CLIENT"

// ADC--------------
// #define ADC_CHANNEL ADC_CHANNEL_2   // GPIO19 for ADC1
// #define DEFAULT_VREF 1100           // Default reference voltage in mV (you may need to adjust this)
// #define NO_OF_SAMPLES 64            // Multisampling to improve accuracy

#define MUSIC_IDLE           0x00
#define MUSIC_1              0x01
#define MUSIC_2              0x02
#define MUSIC_STOPING        0x03
#define MUSIC_STOP           0x04
#define MUSIC_IMMEDIATE_STOP 0X05




//------------------------------------------------------------------

#define DEVICE_VERSION_ID "iF11"// device v
#define WL_FLASH_SIZE      6*1024*1024//2097152//FATFS size

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


#define SYNC_SAVED          0x0A
#define SYNC_SAVED_FAIL     0x0B
#define SYNC_DATA_ERROR     0x0C

#define SYNC_DUPLICATE      0x0D
#define SYNC_DONE           0x0E
#define SYNC_ERROR          0x0F


#define TIME_UPDATE         0x10
#define TIME_FORMET_UPDATE  0x11

#define IMAGE_DELETE_SUC    0x12
#define IMAGE_DELETE_FAIL   0x13

#define LISENCE_UPDATE      0x14




//------FATFs directory 

#define  FACE_DIRECTORY "/fatfs/faces"
#define  ATTENDANCE_DIR  "/fatfs/log"
#define  SYNC_DIR  "/fatfs/sync"

//---------------

// sleep ----------------
#define SLEEP   0x01
#define WAKEUP  0x00
#define RE_INIT 0x02
//-----------------------

//------7seegment-------------------------------------------------
// size in H segment
#define HEIGHT_32 6
#define WIDTH_32 30
// size in V segment
#define HEIGHT_8 26
#define WIDTH_8 8

#define segmentBaseX  73
#define segmentBaseY  58 

// #define SEVENSEGMENT_COLOR 0x07E0//. green
#define SEVENSEGMENT_COLOR 0xf800   // red
// #define SEVENSEGMENT_COLOR 0x003f   //blue

//----------------------------------------------------------------
//-------------------Time value in ms-----------------------------
#define TIMEOUT_50_MS         5
#define TIMEOUT_100_MS        10
#define TIMEOUT_120_MS        12
#define TIMEOUT_150_MS        15
#define TIMEOUT_200_MS        20
#define TIMEOUT_300_MS        30
#define TIMEOUT_500_MS        50
#define TIMEOUT_1000_MS       (100)
#define TIMEOUT_2000_MS       (200)
#define TIMEOUT_3000_MS       (300)
#define TIMEOUT_4000_MS       (400)
#define TIMEOUT_5000_MS       (500)
#define TIMEOUT_6000_MS       (600)
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
//--------------------------------------------------------------
#define HALT while(1);
//-------------------topic--------------------------------------

// #define     PUBLISH_TOPIC         "/app/cloud"
// #define     SUBCRIBE_TOPIC        "/topic/cloud"


#define     PUBLISH_TOPIC         "/app/messages"
#define     SUBCRIBE_TOPIC        "/topic/AA00242829068"
//--------------------------------------------------------------
#define ID_VALID 3
//-----------network status code--------------------------------
#define     WIFI_DISS           0x00
#define     WIFI_CONNECTED      0x01
#define     WSS_CONNECTED       0x02
// #define     STOMP_CONNECTED     0x03
//--------------------------------------------------------------

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
// variable padding--------------------------------------------------------------------------
#define PACKED_STRUCT __attribute__((packed))





#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
