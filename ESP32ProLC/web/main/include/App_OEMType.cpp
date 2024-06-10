/*
 * App_OEMType.cpp
 *
 *  Created on: Apr 24, 2024
 *      Author: sang
 */



EXT_RAM_ATTR char SangReturnLen=0;
EXT_RAM_ATTR char SangReturnStr[100];
unsigned char CommunicationInterface=0;
//#include "App_OEMType.cpp"
extern recognizer_state_t OEMGetFaceImageState(void);
extern void OEMSetFaceImageState(recognizer_state_t state);

void RGB565_OEMStyleLabelprint(camera_fb_t *fb, unsigned  char *str)
{
	OEMStyleLabelprint(fb, str);
}
void SaveData(unsigned    int  sizebytes,unsigned char *Buffer)
{
	OEMSaveData(sizebytes,Buffer);
}
void DataParser(void)
{
	OEMDataParser();
}
void SetFaceImageState(recognizer_state_t state)
{
	OEMSetFaceImageState(state);
}
recognizer_state_t GetFaceImageState(void)
{
	return OEMGetFaceImageState();
}
void SangGetFaceInfoCurrent(unsigned char *Name,unsigned char *ID)
{
	OEMGetCurrentSaveFaceInfor(Name,ID);
}

bool SangNVS_SaveFaceInfo(char *EmployeeName,char *EmployeeID,int FaceID)
{
	return OEMNVS_SaveFaceInfo(EmployeeName,EmployeeID,FaceID);
}

 bool sangNVS_GetFaceInfoCurrent(char *EmployeeName,char *EmployeeID,int FaceID)
 {
	 return OEMNVS_GetFaceInfoCurrent(EmployeeName,EmployeeID,FaceID);
 }
void SangBLEReturnValue(bool state,char value)
{
	BLEReturnValue(state,value);
}

void DisplayFreeMemory(char *str)
{
	printf("--------------- heap free size PSRAM %s:%d,total size:%d\r\n", str,(int)heap_caps_get_free_size( MALLOC_CAP_SPIRAM ),
	heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
	printf("--------------- heap free size in processor %s:%ld,total size:%d\r\n",str,(long)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
	heap_caps_get_total_size(MALLOC_CAP_INTERNAL));

}
