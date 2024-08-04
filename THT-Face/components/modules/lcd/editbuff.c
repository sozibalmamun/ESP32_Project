
#include "editbuff.h"
#include "logo&Icon.h"

#define WIFI_WIDTH 16
#define WIFI_HEIGHT 15
#define NETWORK_ICON_POSS_X 295
#define NETWORK_ICON_POSS_Y 3

#define CHAR_WIDTH 8//910
#define CHAR_HEIGHT 10//13



#define LETTER_WIDTH 9//910
#define LETTER_HEIGHT 10//13





uint8_t wifiStatus;
// TickType_t sleep;= xTaskGetTickCount()

const uint8_t *font_table[128] = {

    ['0'] = char_0,
    ['1'] = char_1,
    ['2'] = char_2,
    ['3'] = char_3,
    ['4'] = char_4,
    ['5'] = char_5,
    ['6'] = char_6,
    ['7'] = char_7,
    ['8'] = char_8,
    ['9'] = char_9,
    ['-'] = charSignNeg

    // ['0'] = char_0,
    // ['0'] = char_0,
    // ['0'] = char_0,
};

const uint16_t *font_table1[128] = {

    ['N'] = char_N,
    ['S'] = char_S

};


void editDisplayBuff(camera_fb_t **buff){


    if(wifiStatus==0){

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
    }else 
    {
        iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y,WIFI_WIDTH,WIFI_HEIGHT,&wifiIcon,*buff);

        if(wifiStatus==2){

            iconPrint(NETWORK_ICON_POSS_X+15,NETWORK_ICON_POSS_Y+9,7,7 ,&connectedIcon,*buff);

        }else{

            iconPrint(NETWORK_ICON_POSS_X+15,NETWORK_ICON_POSS_Y+9,2,7,&disconnectedIcon,*buff);

        }
    }

    WriteString(200, 210, "SN-0123456789",*buff);
    // wrightChar(200, 200, '1', *buff);


}

void iconPrint(int x_offset, int y_offset, uint8_t w, uint8_t h,char* logobuff, camera_fb_t *buff) {
    // Ensure logo fits within the buffer dimensions
    if (x_offset + w > buff->width || y_offset + h > buff->height) {
        printf("Logo position out of bounds\n");
        
        return;
    }

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int logo_index = y * w + x;
            int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel

            // Copy the logo pixel to the buffer
            uint8_t pixel = logobuff[logo_index];
            if(pixel==0){// white color
            buff->buf[buff_index] = 0xff;
            buff->buf[buff_index + 1] = 0xff;
            }else if(pixel==2){// red color //
            buff->buf[buff_index] = 0xc0;
            buff->buf[buff_index + 1] = 0x00;
            }else if(pixel==3){//green color 0x3fe5
            buff->buf[buff_index] = 0x3f;
            buff->buf[buff_index + 1] = 0xe5;
            }

        }
    }
}


// Function to render a string onto the display buffer
void WriteString(int x_offset, int y_offset, const char *str, camera_fb_t *buff) {
    while (*str) {
        wrightChar(x_offset, y_offset, *str, buff);
        x_offset += CHAR_WIDTH; // Move to the next character position
        str++;
    }
}

// Function to render a character onto the display buffer
void wrightChar(int x_offset, int y_offset, char c, camera_fb_t *buff) {
    


    if((c>'@'&& c<'[') ||(c>0x60 && c<'{') ){

        // Get the bitmap data for the character
        const uint16_t *char_data = font_table1[(uint8_t)c];

        if (char_data == NULL) {
            printf("Character '%c' not supported\n", c);
            return;
        }

        // Ensure the character fits within the buffer dimensions
        if (x_offset + LETTER_WIDTH > buff->width || y_offset + LETTER_WIDTH > buff->height) {
            printf("Character position out of bounds\n");
            return;
        }

        for (int y = 0; y < LETTER_HEIGHT; y++) {
            for (int x = 0; x <= LETTER_WIDTH; x++) {

                int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel
                // Get the pixel value from the character data
                if (char_data[y] & (1 << (LETTER_WIDTH-x))) {
                    // Draw white (pixel set)
                    buff->buf[buff_index] = 0xFF;
                    buff->buf[buff_index + 1] = 0xFF;
                }
            }
        }
    }else{

        // Get the bitmap data for the character
        const uint8_t *char_data = font_table[(uint8_t)c];
        if (char_data == NULL) {
            printf("Character '%c' not supported\n", c);
            return;
        }
        // Ensure the character fits within the buffer dimensions
        if (x_offset + CHAR_WIDTH > buff->width || y_offset + CHAR_WIDTH > buff->height) {
            printf("Character position out of bounds\n");
            return;
        }

        for (int y = 0; y < CHAR_HEIGHT; y++) {
            for (int x = 0; x <= CHAR_WIDTH; x++) {
                int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel
                // Get the pixel value from the character data
                if (char_data[y] & (1 << (CHAR_WIDTH-x))) {
                    // Draw white (pixel set)
                    buff->buf[buff_index] = 0xFF;
                    buff->buf[buff_index + 1] = 0xFF;
                }
            }
        }
    }

}
