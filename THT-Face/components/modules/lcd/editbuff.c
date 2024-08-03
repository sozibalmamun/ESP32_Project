
#include "editbuff.h"
#include "logo&Icon.h"

#define WIFI_WIDTH 16
#define WIFI_HEIGHT 15
#define NETWORK_ICON_POSS_X 295
#define NETWORK_ICON_POSS_Y 3

#define CHAR_WIDTH 9
#define CHAR_HEIGHT 16

uint8_t wifiStatus;

const uint16_t *font_table[128] = {
    ['A'] = char_A,
    ['0'] = zero,
    // Add more characters here...
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

    wrightChar(200,210,'0',*buff);


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


// Function to render a character onto the display buffer
void wrightChar(int x_offset, int y_offset, char c, camera_fb_t *buff) {
    // Get the bitmap data for the character
    const uint16_t *char_data = font_table[(uint8_t)c];
    if (char_data == NULL) {
        printf("Character '%c' not supported\n", c);
        return;
    }

    // Ensure the character fits within the buffer dimensions
    if (x_offset + CHAR_WIDTH > buff->width || y_offset + CHAR_HEIGHT > buff->height) {
        printf("Character position out of bounds\n");
        return;
    }

    for (int y = 0; y < CHAR_HEIGHT; y++) {
        for (int x = 0; x < CHAR_WIDTH; x++) {
            int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel

            // Get the pixel value from the character data
            if (char_data[y] & (1 << (CHAR_WIDTH - 1 - x))) {
                // Draw black (pixel set)
                buff->buf[buff_index] = 0xFF;
                buff->buf[buff_index + 1] = 0xFF;
            }
        }
    }
}


// // Function to render a character onto the display buffer
// void wrightChar(int x_offset, int y_offset, const uint8_t *char_data, camera_fb_t *buff) {
//     // Ensure the character fits within the buffer dimensions
//     if (x_offset + CHAR_WIDTH > buff->width || y_offset + CHAR_HEIGHT > buff->height) {
//         printf("Character position out of bounds\n");
//         return;
//     }

//     for (int y = 0; y < CHAR_HEIGHT; y++) {
//         for (int x = 0; x < CHAR_WIDTH; x++) {
//             int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel

//             // Get the pixel value from the character data
//             if (char_data[y] & (1 << (CHAR_WIDTH - 1 - x))) {
//                 // Draw black (pixel set)
//                 buff->buf[buff_index] = 0xFF;
//                 buff->buf[buff_index + 1] = 0xFF;
//             }
//         }
//     }
// }










// void drawChar(uint16_t x, uint16_t y, uint16_t letter, uint16_t color, uint16_t height, uint16_t width)
// {
// 	unsigned char row, col, bytes, temp, bytesPerRow;
	
// 	y += (uint16_t)(lineNumber * height);
	
// 	bytesPerRow = ceil((double)width / 8);
	
// 	for (row = 0; row < height; row++ ) {
		
// 		for (bytes = bytesPerRow; bytes > 0 ; bytes--) {
			
// 			temp = pgm_read_byte_far(line + letter + row * bytesPerRow + bytes - 1 );
			
// 			unsigned char bitsExtra = 0;
// 			if(bytes == bytesPerRow) 
// 			{
// 				bitsExtra =  bytesPerRow * 8 - width;
// 				temp >>= bitsExtra;
// 			}
// 			for (col = 0; col < 8 - bitsExtra; col++) {
// 				if (temp & 0x01) {
// 					ST7735_drawPixel(x + ( 8 * bytes - bitsExtra ) - col, y + row, color);
// 				}
// 				temp >>= 1;
// 			}
// 		}
// 	}
// }