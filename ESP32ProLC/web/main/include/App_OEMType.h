/*
 * App_OEMType.h
 *
 *  Created on: Nov 20, 2023
 *      Author: sang
 */

#include <stdint.h>
#include "esp_log.h"
#include "who_camera.h"





#ifdef __cplusplus
extern "C" {
#endif
extern EXT_RAM_ATTR char SangReturnLen;
extern EXT_RAM_ATTR char SangReturnStr[];
extern unsigned char CommunicationInterface;
extern EXT_RAM_ATTR unsigned int FaceInfoQuantity;


#define MAXSocketClientRevSize	1000
typedef struct
{
	volatile bool WifiConnectedState;
	volatile int sockID;
	volatile char HostIPAddress[20];
	volatile unsigned int Port;
	volatile bool SocketClientConnectedState;
	volatile bool SocketClientTaskRuning;
	volatile char ReceiveBuffer[MAXSocketClientRevSize];

}SocketClientInstance;


void Tcp_Client_Connect_Task(void *pvParameters);

typedef struct
{
	volatile bool WifiConnectState;		//wifi connnect state 1:connected 0:disconnected
	uint32_t CurrentStationIP;			//Current IP when connected
	int WifiReConnnectTimes;			//When loss connection the max reconnect times.
	char WifiName[64];					//wifi name
	char WifiPassword[64];				//wifi password

}WIFIInfo;


void app_wifi_main_sang(void);
void TCP_Client_SendData(char *BufferSend,unsigned int Slen);


void TestNVS(void);
void App_Ble_Start(void);


void LVGL_app_main(void);
void register_camera_LVGL(const pixformat_t pixel_fromat,const framesize_t frame_size,const uint8_t fb_count,const QueueHandle_t frame_o);
esp_err_t register_lcd_sang(const QueueHandle_t frame_i, const QueueHandle_t frame_o, const bool return_fb);

void DisplayFreeMemory(char *str);

void SaveData(unsigned	 int  sizebytes,unsigned char *Buffer);
void DataParser(void);
bool NVS_SaveFaceInfo(char *EmployeeName,char *EmployeeID,int FaceID);
void GetGBKCode(unsigned char  * pBuffer,unsigned char * c);
void GetASCIICode(unsigned char * pBuffer,unsigned char ASCII);
void OEMStyleLabelprint(camera_fb_t *fb, unsigned char *str);


void OEMDataParser(void);
void OEMSaveData(unsigned	 int  sizebytes,unsigned char *Buffer);
void IntialParser(void);
void OEMGetCurrentSaveFaceInfor(unsigned char *Name,unsigned char *ID);


bool OEMNVS_SaveFaceInfo(char *EmployeeName,char *EmployeeID,int FaceID);
bool OEMNVS_GetFaceInfoCurrent(char *EmployeeName,char *EmployeeID,int FaceID);
bool NVS_GetWifiSSIDAndPassword(char *PWifiName,char *PWifiPassword);
bool NVS_SetWifiSSIDAndPassword(char *WifiName,char *Password);
bool NVS_GetNetWorkConfirm(char *IPStr,unsigned int *Port);
bool NVS_SaveNetWorkConfirm(char *IPStr,unsigned int Port);
void NVS_SaveFaceinfoQuantity(void);
void NVS_DeleteALLFaceInformation(void);


void SetComunicationInterface(unsigned char Type);
void OEMSetComunicationInterface(unsigned char Type);
void SangBLEReturnValue(bool state,char value);
void BLEReturnValue(bool state,char value);

	


#ifdef __cplusplus
}
#endif


