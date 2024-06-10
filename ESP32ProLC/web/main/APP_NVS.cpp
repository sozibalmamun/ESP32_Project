/*
 * NVS_WT.cpp
 *
 *  Created on: Mar 4, 2024
 *      Author: sang
 */
#include "who_camera.h"
#include "who_button.h"
#include "who_adc_button.h"
#include "who_human_face_recognition.hpp"
#include "app_wifi.h"
#include "app_httpd.hpp"
#include "app_mdns.h"
#include "who_lcd.h"
#include "App_OEMType.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_bt.h"
#include "string.h"
#include "stdio.h"


 EXT_RAM_ATTR unsigned int FaceInfoQuantity=0;	

bool NVS_SetWifiSSIDAndPassword(char *WifiName,char *Password)
{
	// Open
	printf("Opening Non-Volatile Storage (NVS) handle... \r\n");
	nvs_handle_t my_handle;
	esp_err_t err = nvs_open("nvs", NVS_READWRITE, &my_handle);
	
	if (err != ESP_OK) 
	{
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		return false;
	} 
	printf("sang:Nvs_open success!\r\n");
	
	err = nvs_set_str(my_handle, "WiFi_SSID", WifiName);
	if (err != ESP_OK) 
	{
		printf("Error (%s) nvs_set_str WiFi_SSID!\n", esp_err_to_name(err));
		nvs_close(my_handle);
		return false;
	} 
	printf("sang:nvs_set_str WiFi_SSID success!\r\n");
	
	err = nvs_set_str(my_handle, "WiFi_PASSWORD", Password);
	if (err != ESP_OK) 
	{
		printf("sang:Error (%s) nvs_set_str WiFi_PASSWORD!\n", esp_err_to_name(err));
		nvs_close(my_handle);
		return false;
	} 
	printf("sang:nvs_set_str WiFi_PASSWORD success!\r\n");
	err = nvs_commit(my_handle);
	if (err != ESP_OK) 
	{
		printf("Error (%s) nvs_set_str WiFi_PASSWORD!\n", esp_err_to_name(err));
		nvs_close(my_handle);
		return false;
	}
	return true;
	// Close
    nvs_close(my_handle);
	
}
bool NVS_GetWifiSSIDAndPassword(char *PWifiName,char *PWifiPassword)
{
	// Open
	printf("Opening Non-Volatile Storage (NVS) handle... \r\n");
	nvs_handle_t my_handle;
	esp_err_t err = nvs_open("nvs", NVS_READONLY, &my_handle);
	if (err != ESP_OK) 
	{
		   printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		   return false;
	} 
	printf("sang:Nvs_open success!\r\n");
	
	//read WiFi_SSID
	size_t required_size;
	err=nvs_get_str(my_handle, "WiFi_SSID", NULL, &required_size);
	if (err != ESP_OK) 
	{
		printf("sang:Error (%s) nvs_get_str size!\r\n", esp_err_to_name(err));
		nvs_close(my_handle);
		return false;
	}
	printf("sang:WiFi_SSID:%d\r\n",required_size);
	char* wifiname = (char*)malloc(required_size);
	err=nvs_get_str(my_handle, "WiFi_SSID", wifiname, &required_size);
	if (err != ESP_OK) 
	{
		printf("Error (%s) nvs_get_str wifiname!\r\n", esp_err_to_name(err));
		nvs_close(my_handle);
		return false;
	}
	printf("sang:WiFi_SSID:%s\r\n",wifiname);
	strcpy(PWifiName,wifiname);
	free(wifiname);

	//read WiFi_SSID password
	//size_t required_size;
	err=nvs_get_str(my_handle, "WiFi_PASSWORD", NULL, &required_size);
	if (err != ESP_OK) 
	{
		printf("Error (%s) nvs_get_str WiFi_PASSWORD size!\r\n", esp_err_to_name(err));
		nvs_close(my_handle);
		return false;
	}
	printf("sang:WiFi_PASSWORD:%d\r\n",required_size);
	char* wifipassword = (char*)malloc(required_size);
	err=nvs_get_str(my_handle, "WiFi_PASSWORD", wifipassword, &required_size);
	if (err != ESP_OK) 
	{
		printf("Error (%s) nvs_get_str WiFi_PASSWORD!\r\n", esp_err_to_name(err));
		nvs_close(my_handle);
		return false;
	}
	printf("sang:WiFi_PASSWORD:%s\r\n",wifipassword);
	strcpy(PWifiPassword,wifipassword);
	free(wifipassword);

	// Close
    nvs_close(my_handle);

	return true;

}
bool NVS_GetFaceinfoQuantity(void)
{
	// Open
	printf("Opening Non-Volatile Storage (NVS) handle... \r\n");
	nvs_handle_t my_handle;
	esp_err_t err = nvs_open("nvs", NVS_READONLY, &my_handle);
	if (err != ESP_OK) 
	{
		   printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		   return false;
	} 
	printf("sang:Nvs_open success!\r\n");

	//read face info quantity
	uint16_t  Value;
	err=nvs_get_u16(my_handle, "FaceIQ",&Value);
	if (err != ESP_OK) 
	{
		printf("sang:Error (%s) get Face_info_quantity!\r\n", esp_err_to_name(err));
		nvs_close(my_handle);
		FaceInfoQuantity=0;
		return false;
	}
	FaceInfoQuantity=Value;
	printf("sang:Face_info_quantity:%d\r\n",Value);	
	
	// Close
    nvs_close(my_handle);
	return true;

}
void NVS_SaveFaceinfoQuantity(void)
{

	// Open
	printf("Opening Non-Volatile Storage (NVS) handle... \r\n");
	nvs_handle_t my_handle;
	esp_err_t err = nvs_open("nvs", NVS_READWRITE, &my_handle);
	
	if (err != ESP_OK) 
	{
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		return;
	} 
	printf("sang:Nvs_open success!\r\n");

	err = nvs_set_u16(my_handle, "FaceIQ", FaceInfoQuantity);
	if (err != ESP_OK) 
	{
		printf("Error (%s) nvs_set_u16 FaceInfoQuantity!\n", esp_err_to_name(err));
		nvs_close(my_handle);
		return;
	} 
	
	err = nvs_commit(my_handle);
	if (err != ESP_OK) 
	{
		printf("Error (%s) nvs_commit FaceInfoQuantity!\n", esp_err_to_name(err));
		nvs_close(my_handle);
		return;
	}
	printf("sang:FaceInfoQuantity:%d\r\n",FaceInfoQuantity);
	
	// Close
    nvs_close(my_handle);

}

static EXT_RAM_ATTR char EmployeeInfor_Key[10];
static EXT_RAM_ATTR char EmployeeInfor[50];
bool OEMNVS_SaveFaceInfo(char *EmployeeName,char *EmployeeID,int FaceID)
{
	printf("OEMNVS_SaveFaceInfo:%s,%s,%d\r\n",EmployeeName,EmployeeID,FaceID);
	//---step 1 get the quantity of total face information---
	if(false==NVS_GetFaceinfoQuantity())
	{
		printf("NVS_GetFaceinfoQuantity Err!\n");
		NVS_SaveFaceinfoQuantity();
		BLEReturnValue(false,2);
		return false;
	}
	//---step 2 create new face info nvs Key for save---
	sprintf(EmployeeInfor_Key,"FA%04d",FaceInfoQuantity);
	printf("EmployeeInfor_Key:%s\r\n",EmployeeInfor_Key);
	//---step 3 create face infor string--
	sprintf(EmployeeInfor,"%s %s %d\n",EmployeeName,EmployeeID,FaceID);
	printf("EmployeeInfor:%s",EmployeeInfor);

	
	// Open
	printf("Opening Non-Volatile Storage (NVS) handle... \r\n");
	nvs_handle_t my_handle;
	esp_err_t err = nvs_open("nvs", NVS_READWRITE, &my_handle);
	if (err != ESP_OK) 
	{
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		BLEReturnValue(false,2);
		return false;
	} 
	printf("sang:Nvs_open success!\r\n");
	
	err = nvs_set_str(my_handle, (const char*)EmployeeInfor_Key, EmployeeInfor);
	if (err != ESP_OK) 
	{
		printf("Error (%s) nvs_set_str EmployeeInfor!\n", esp_err_to_name(err));
		nvs_close(my_handle);
		BLEReturnValue(false,2);
		return false;
	} 
	
	err = nvs_commit(my_handle);
	if (err != ESP_OK) 
	{
		printf("Error (%s) nvs_set_str EmployeeInfor!\n", esp_err_to_name(err));
		nvs_close(my_handle);
		BLEReturnValue(false,2);
		return false;
	}
	printf("sang:nvs_set_str EmployeeInfor success!\r\n");
	// Close
    nvs_close(my_handle);

	FaceInfoQuantity++;
	NVS_SaveFaceinfoQuantity();
	BLEReturnValue(true,2);
    return true;
}
//--------Search for employee name through facial information ID.
bool OEMNVS_GetFaceInfoCurrent(char *EmployeeName,char *EmployeeID,int FaceID)
{
	//---step 1 get the quantity of total face information---
	if(false==NVS_GetFaceinfoQuantity())
	{
		printf("NVS_GetFaceinfoQuantity Err!\n");
		return false;
	}
	// Open
	printf("Opening Non-Volatile Storage (NVS) handle... \r\n");
	nvs_handle_t my_handle;
	esp_err_t err = nvs_open("nvs", NVS_READONLY, &my_handle);
	if (err != ESP_OK) 
	{
		   printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		   return false;
	} 
	printf("sang:Nvs_open success!\r\n");

	size_t required_size;
	unsigned int PwriteT=0;
	unsigned int PReadT=0;

	char TempEmployeeName[20],TempEmployeeID[20],TempEmployeeFaceID[20];
	int TempID;
	bool ChineseGBK=false;
	char step=0;
	//printf("sang:find face ID:%d,FaceInfoQuantity:%d\r\n",FaceID,FaceInfoQuantity);
	for(int i=0;i<FaceInfoQuantity;i++)
	{
		//read face info 
		//size_t required_size;
		sprintf(EmployeeInfor_Key,"FA%04d",i);
		err=nvs_get_str(my_handle, (const char*)EmployeeInfor_Key, NULL, &required_size);
		if (err != ESP_OK) 
		{
			printf("Error (%s) nvs_get_str %s err!\r\n", esp_err_to_name(err),EmployeeInfor_Key);
			nvs_close(my_handle);
			return false;
		}
		char* TempStr = (char*)malloc(required_size);
		err=nvs_get_str(my_handle, (const char*)EmployeeInfor_Key, TempStr, &required_size);
		if (err != ESP_OK) 
		{
			printf("Error (%s) nvs_get_str EmployeeInfor!\r\n", esp_err_to_name(err));
			nvs_close(my_handle);
			return false;
		}
		//printf("sang:EmployeeInfor:%s\r\n",TempStr);
		//compare ID
		PwriteT=0;
		PReadT=0;
		step=0;
		while(*(TempStr+PReadT)!='\0')
		{
			if(0==step)//employeeName
			{
				if(ChineseGBK==false)
				{
					if(' '==*(TempStr+PReadT))
					{
						TempEmployeeName[PwriteT]='\0';
						PwriteT=0;
						step=1;
						printf("TempEmployeeName:%s\r\n",TempEmployeeName);
						PReadT++;
					}
					else
					{
						TempEmployeeName[PwriteT++]=*(TempStr+PReadT);
						if(*(TempStr+PReadT)>=0xa1&&*(TempStr+PReadT)<=0xf7)
						{
							ChineseGBK=true;
						}
						PReadT++;
					}
				}
				else
				{
					TempEmployeeName[PwriteT++]=*(TempStr+PReadT);
					ChineseGBK=false;
					PReadT++;
				}
			}
			else if(1==step)
			{
				if(' '==*(TempStr+PReadT))
				{
					TempEmployeeID[PwriteT]='\0';
					PwriteT=0;
					step=2;
					//printf("TempEmployeeID:%s\r\n",TempEmployeeID);
					PReadT++;
				}
				else
				{
					TempEmployeeID[PwriteT++]=*(TempStr+PReadT);
					PReadT++;
				}				
			}
			else if(2==step)
			{
				if(0x0a==*(TempStr+PReadT))
				{
					TempEmployeeFaceID[PwriteT]='\0';
					PwriteT=0;
					printf("TempEmployeeFaceID:%s\r\n",TempEmployeeFaceID);
					TempID=atoi(TempEmployeeFaceID);
					//printf("TempID:%d\r\n",TempID);
					if(TempID==FaceID)
					{
						printf("sang:Find face ID:%d\r\n",FaceID);
						strcpy(EmployeeName,TempEmployeeName);
						strcpy(EmployeeID,TempEmployeeID);
						free(TempStr);
						nvs_close(my_handle);
						return true;
					}
					break;
				}
				else
				{
					TempEmployeeFaceID[PwriteT++]=*(TempStr+PReadT);
					PReadT++;
				}				
			}			
		}
		
		
		free(TempStr);
	
	}
	
	// Close
    nvs_close(my_handle);
	return false;
}
bool NVS_GetFaceInfoTotal(void)
{
	//---step 1 get the quantity of total face information---
	if(false==NVS_GetFaceinfoQuantity())
	{
		printf("NVS_GetFaceinfoQuantity Err!\n");
		return false;
	}
	// Open
	printf("Opening Non-Volatile Storage (NVS) handle... \r\n");
	nvs_handle_t my_handle;
	esp_err_t err = nvs_open("nvs", NVS_READONLY, &my_handle);
	if (err != ESP_OK) 
	{
		   printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		   return false;
	} 
	printf("sang:Nvs_open success!\r\n");

	size_t required_size;
	unsigned int PwriteT=0;
	unsigned int PReadT=0;

	char TempEmployeeName[20],TempEmployeeID[20],TempEmployeeFaceID[20];
	int TempID;
	bool ChineseGBK=false;
	char step=0;
	for(int i=0;i<FaceInfoQuantity;i++)
	{
		//read face info 
		//size_t required_size;
		sprintf(EmployeeInfor_Key,"FA%04d",i);
		err=nvs_get_str(my_handle, (const char*)EmployeeInfor_Key, NULL, &required_size);
		if (err != ESP_OK) 
		{
			printf("Error (%s) nvs_get_str %s err!\r\n", esp_err_to_name(err),EmployeeInfor_Key);
			nvs_close(my_handle);
			return false;
		}
		char* TempStr = (char*)malloc(required_size);
		err=nvs_get_str(my_handle, (const char*)EmployeeInfor_Key, TempStr, &required_size);
		if (err != ESP_OK) 
		{
			printf("Error (%s) nvs_get_str EmployeeInfor!\r\n", esp_err_to_name(err));
			nvs_close(my_handle);
			return false;
		}
		printf("sang:EmployeeInfor:%s\r\n",TempStr);
		//compare ID
		PwriteT=0;
		PReadT=0;
		step=0;
		while(*(TempStr+PReadT)!='\0')
		{
			if(0==step)//employeeName
			{
				if(ChineseGBK==false)
				{
					if(' '==*(TempStr+PReadT))
					{
						TempEmployeeName[PwriteT]='\0';
						PwriteT=0;
						step=1;
						printf("TempEmployeeName:%s\r\n",TempEmployeeName);
						PReadT++;
					}
					else
					{
						TempEmployeeName[PwriteT++]=*(TempStr+PReadT);
						if(*(TempStr+PReadT)>=0xa1&&*(TempStr+PReadT)<=0xf7)
						{
							ChineseGBK=true;
						}
						PReadT++;
					}
				}
				else
				{
					TempEmployeeName[PwriteT++]=*(TempStr+PReadT);
					ChineseGBK=false;
					PReadT++;
				}
			}
			else if(1==step)
			{
				if(' '==*(TempStr+PReadT))
				{
					TempEmployeeID[PwriteT]='\0';
					PwriteT=0;
					step=2;
					printf("TempEmployeeID:%s\r\n",TempEmployeeID);
					PReadT++;
				}
				else
				{
					TempEmployeeID[PwriteT++]=*(TempStr+PReadT);
					PReadT++;
				}				
			}
			else if(2==step)
			{
				if(0x0a==*(TempStr+PReadT))
				{
					TempEmployeeFaceID[PwriteT]='\0';
					PwriteT=0;
					printf("TempEmployeeFaceID:%s\r\n",TempEmployeeFaceID);
					TempID=atoi(TempEmployeeFaceID);
					printf("TempID:%d\r\n",TempID);
					PReadT++;
					break;
				}
				else
				{
					TempEmployeeFaceID[PwriteT++]=*(TempStr+PReadT);
					PReadT++;
				}				
			}			
		}
		
		
		free(TempStr);
	
	}
	
	// Close
    nvs_close(my_handle);
	return false;
}
static EXT_RAM_ATTR char NetString[60];
bool NVS_SaveNetWorkConfirm(char *IPStr,unsigned int Port)
{
	nvs_handle_t my_handle;
	esp_err_t err = nvs_open("nvs", NVS_READWRITE, &my_handle);
	
	if (err != ESP_OK) 
	{
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		return false;
	} 
	printf("sang:Nvs_open success!\r\n");


	sprintf(NetString,"%s %d\n",IPStr,Port);
	printf("sang:NVS save str:%s\r\n",NetString);
	err = nvs_set_str(my_handle, "Net", NetString);
	if (err != ESP_OK) 
	{
		printf("Error (%s) nvs_set_str net!\n", esp_err_to_name(err));
		nvs_close(my_handle);
		return false;
	} 
	err = nvs_commit(my_handle);
	if (err != ESP_OK) 
	{
		printf("Error (%s) nvs_commit net!\n", esp_err_to_name(err));
		nvs_close(my_handle);
		return false;
	}
	// Close
    nvs_close(my_handle);
    return true;

}
bool NVS_GetNetWorkConfirm(char *IPStr,unsigned int *Port)
{
	nvs_handle_t my_handle;
	esp_err_t err = nvs_open("nvs", NVS_READONLY, &my_handle);
	if (err != ESP_OK) 
	{
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		return false;
	} 
	printf("sang:Nvs_open success!\r\n");

	size_t required_size;
	unsigned int PwriteT=0;
	unsigned int PReadT=0;

	char IPStrTemp[30],PortStrTemp[30];
	int TempID=0;
	char step=0;
	err=nvs_get_str(my_handle, "Net", NetString, &required_size);
	if (err != ESP_OK) 
	{
		printf("Error (%s) nvs_get_str %s err!\r\n", esp_err_to_name(err),EmployeeInfor_Key);
		nvs_close(my_handle);
		return false;
	}
	//compare ID
	PwriteT=0;
	PReadT=0;
	step=0;
	while(*(NetString+PReadT)!='\0')
	{
		if(0==step)//employeeName
		{
			if(' '==*(NetString+PReadT))
			{
				IPStrTemp[PwriteT]='\0';
				PwriteT=0;
				step=1;
				TempID=0;
				printf("IPStrTemp:%s\r\n",IPStrTemp);
			}
			else
			{
				IPStrTemp[PwriteT++]=*(NetString+PReadT);
				if(PwriteT>30)
				{
					nvs_close(my_handle);
					return false;
				}
			}
		}
		else if(1==step)
		{
			if('\n'==*(NetString+PReadT))
			{
				PortStrTemp[PwriteT]='\0';
				printf("PortStrTemp:%s,TempID:%d\r\n",PortStrTemp,TempID);
				break;
			}
			else
			{
				PortStrTemp[PwriteT++]=*(NetString+PReadT);		
				if(PwriteT>30)
				{
					nvs_close(my_handle);
					return false;
				}
				TempID=TempID*10+*(NetString+PReadT)-'0';
			}		
		}
		PReadT++;
	}
	
	strcpy(IPStr,IPStrTemp);
	*Port=TempID;
	// Close
    nvs_close(my_handle);
	return false;
}
void NVS_DeleteALLFaceInformation(void)
{
	if(false==NVS_GetFaceinfoQuantity())
	{
		printf("Error NVS_GetFaceinfoQuantity!\n");
		return;
	}
	nvs_handle_t my_handle;
	esp_err_t err = nvs_open("nvs", NVS_READWRITE, &my_handle);
	
	if (err != ESP_OK) 
	{
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		return;
	} 
	printf("sang:Nvs_open success!\r\n");
	nvs_erase_all(my_handle);
	if (err != ESP_OK) 
	{
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		return;
	}
	// Close
    nvs_close(my_handle);

}

void TestNVS(void)
{
	NVS_GetFaceinfoQuantity();
	NVS_GetFaceInfoTotal();


}

