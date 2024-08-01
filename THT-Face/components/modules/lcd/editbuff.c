
#include "editbuff.h"


uint8_t wifiStatus=false;

void editDisplayBuff(camera_fb_t **buff){

    if(!wifiStatus){

        for (int y = qrInfo.yOfset-3; y < qrInfo.yOfset-3 + qrInfo.erase_size; y++)
        {
            for (int x = qrInfo.xOfset-3; x < qrInfo.xOfset-3 + qrInfo.erase_size; x++)
            {
                int index = (y * (*buff)->width + x) * 2; // Assuming 2 bytes per pixel

                (*buff)->buf[index] = 0xff;
                (*buff)->buf[index + 1] = 0xff;
            
            }
        }

        char tempFrame[10] ;
        snprintf(tempFrame, sizeof(tempFrame), "%09llu", generate_unique_id());
        createQrcode(tempFrame , *buff);
    }

}
