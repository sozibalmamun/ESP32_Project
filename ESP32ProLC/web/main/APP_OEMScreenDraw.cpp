/*
 * OEMScreenDraw.c
 *
 *  Created on: Apr 22, 2024
 *      Author: sang
 */
#include "who_camera.h"
#include "who_button.h"
#include "who_adc_button.h"
#include "app_wifi.h"
#include "app_httpd.hpp"
#include "app_mdns.h"
#include "who_lcd.h"
#include "App_OEMType.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_bt.h"
#include "string.h"


#define RGB565Width 	320
#define RGB565Height	240
#define RGB565ImageSize	153600
const unsigned char CMP8[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
static unsigned char bufDecoding[72];
void OEMStyleLabelprint(camera_fb_t *fb, unsigned char *str)
{
	unsigned int W,H,OffsetBytes,WBytes;
	unsigned char GBCodingBytes[2],DecodingSteps;
	unsigned int i,j,k;
	unsigned int OffSetDB,offsetIDB;
	unsigned int x=0,y=0;
	if(fb->width!=RGB565Width||fb->height!=RGB565Height||fb->len!=RGB565ImageSize)
	{
		printf("Image format err!\r\n");
		return;
	}
	//calculate the Write buffer size
	unsigned int WriteWidthDots;
	OffsetBytes=0;
	WriteWidthDots=0;
	while(*(str+OffsetBytes)!='\0')
	{
		if(0xB0<=*(str+OffsetBytes)&&*(str+OffsetBytes)<=0xf7)//GB2312 coding
		{
			WriteWidthDots+=24;
			OffsetBytes++;
		}
		else if((*(str+OffsetBytes) >= 0x20)&&(*(str+OffsetBytes) <=0x7F))//ASCII coding
		{
			WriteWidthDots+=12;
		}
		OffsetBytes++;
	}
	//printf("WriteWidthDots:%d\r\n",WriteWidthDots);
	if(WriteWidthDots<320)
	{
		x=(320-WriteWidthDots)/2;	//display in the middle of the screen
		y=210;
	}
	else
	{
		x=0;	//display in the middle of the screen
		y=216;
	}
	
	DecodingSteps=0;
	WBytes=RGB565Width*2;

	OffsetBytes=0;
	while(*(str+OffsetBytes)!='\0')
	{
		switch(DecodingSteps)
		{
			case 0:
				if(0xB0<=*(str+OffsetBytes)&&*(str+OffsetBytes)<=0xf7)//GB2312 coding
				{
					GBCodingBytes[0]=*(str+OffsetBytes);// Save the first bytes of GBCoding
					DecodingSteps=1;
					
				}
				else if((*(str+OffsetBytes) >= 0x20)&&(*(str+OffsetBytes) <=0x7F))//ASCII coding
				{
					GetASCIICode(bufDecoding,*(str+OffsetBytes));
					//write RGB565 image
					if(x>320||y>240)
					{
						printf("Coordinate err!\r\n");
						return;
					}

					for(j=0;j<24;j++)
					{
						offsetIDB=(y+j)*WBytes+x*2;//current start fill location of the image
						for(i=0;i<2;i++)
						{
							OffSetDB=j*2+i;//current location of decoding buffer offset
							for(k=0;k<8/(i+1);k++)
							{
								if(bufDecoding[OffSetDB]&CMP8[k])
								{
									fb->buf[offsetIDB++]=0xff;
									fb->buf[offsetIDB++]=0xff;
								}
								else
								{
									fb->buf[offsetIDB++]=0x00;
									fb->buf[offsetIDB++]=0x1F;
								}

							}
						}
					}
					x+=12;
				}
			break;
			case 1:
				GBCodingBytes[1]=*(str+OffsetBytes);// Save the first bytes of GBCoding
				DecodingSteps=0;
				GetGBKCode(bufDecoding,GBCodingBytes);
				//write RGB565 image
				if(x>320||y>240)
				{
					printf("Coordinate err!\r\n");
					return;
				}
				for(j=0;j<24;j++)
				{
					offsetIDB=(y+j)*WBytes+x*2;//current start fill location of the image
					for(i=0;i<3;i++)
					{
						OffSetDB=j*3+i;//current location of decoding buffer offset
						for(k=0;k<8;k++)
						{
							if(bufDecoding[OffSetDB]&CMP8[k])
							{
								fb->buf[offsetIDB++]=0xff;
								fb->buf[offsetIDB++]=0xff;
							}
							else
							{
								fb->buf[offsetIDB++]=0x00;
								fb->buf[offsetIDB++]=0x1F;
							}
						}
					}
				}
				x+=24;
			break;
		}
		OffsetBytes++;
	}

}

