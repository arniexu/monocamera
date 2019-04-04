#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

typedef struct __Pixel
{
		unsigned char r;
		unsigned char g;
		unsigned char b;
}pixel;

/**********************************************************************************
EPD Display Colome
            0 --------------------------------------------------------------------- 264
    row     1
            2
            3
            |               EPD Display

	          |
            176 ------------------------------------------------------------------- 264
**********************************************************************************/
#define EPD_HEIGHT 176
#define EPD_WIDTH  264
extern unsigned char disbuf[176][33];

double getGrayLevel(unsigned char r , unsigned char g , unsigned char b);
void ConvertTo1Bit(unsigned char bitmap[][EPD_WIDTH*3],unsigned short npixel);

#endif
