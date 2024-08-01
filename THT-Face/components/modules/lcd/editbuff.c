
#include "editbuff.h"



void editDisplayBuff(camera_fb_t **buff){

// .xOfset = 202,  // Starting x coordinate
// .yOfset = 22, // Starting y coordinate

// .width = 110,
// .height = 110

// int start_x = 180;  // Starting x coordinate
// int start_y = 20;  // Starting y coordinate
// int erase_width = 120;
// int erase_height = 120;


    int start_x = 180;  // Starting x coordinate
    int start_y = 20;  // Starting y coordinate
    int erase_width = 93;
    int erase_height = 93;


    for (int y = start_y; y < start_y + erase_height; y++)
    {
        for (int x = start_x; x < start_x + erase_width; x++)
        {
            int index = (y * (*buff)->width + x) * 2; // Assuming 2 bytes per pixel

            (*buff)->buf[index] = 0xff;
            (*buff)->buf[index + 1] = 0xff;
        

        }
    }

    createQrcode("sozib" , *buff);


}