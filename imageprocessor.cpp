#include "imageprocessor.h"

double getGrayLevel(unsigned char r,unsigned char g,unsigned char b)
{
		return (r * 0.599 + g * 0.287 + b * 0.114) / 255;
}
/*
* @func ConvertTo1Bit
* @param Bitmap image Bitmap[][EPD_WIDTH * 3]
* @note there are EPD_WIDTH pixels per row , and each row consists of three bytes to describe it 
* @ret   Bitmap image
*/
void ConvertTo1Bit(unsigned char Bitmap[][EPD_WIDTH*3],unsigned short nRow)
{
		unsigned char masks[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
		char grayscale[2][EPD_WIDTH]={0};
		unsigned short iRow = 0;  // the ith row piexels of the JPEG image
		unsigned short jColume = 0;  // the jth pixel in i row of the JPEG image
		unsigned short temp = 0;
		do
		{
			if(iRow > 0)
				iRow ++;
			//convert 2 rows of raw JPEG images into grayscale image
			for(temp = iRow ; iRow < EPD_HEIGHT && iRow < temp + 2 ; iRow ++,Bitmap ++)
			{
				for(jColume = 0 ; jColume < EPD_WIDTH ; jColume ++)
				{
					grayscale[iRow][jColume] = (char)(64 * (getGrayLevel(Bitmap[iRow][jColume*3+2],Bitmap[iRow][jColume*3+1],Bitmap[iRow][jColume*3])-0.5));
				}
			}
			iRow -= 2;
			//convert 1 row of the grayscale image into 1bpp bitmap image
			for(temp = iRow ; iRow < EPD_HEIGHT && iRow < temp + 1; iRow ++,Bitmap ++)
			{
				for(jColume = 0 ; jColume < EPD_WIDTH ; jColume ++)
				{
					char error;
					if(grayscale[iRow][jColume] > 0)
							disbuf[iRow][jColume/8] |= masks[jColume%8];
					error = grayscale[iRow][jColume] - (grayscale[iRow][jColume] > 0 ? 32 : -32);
					if (jColume < EPD_WIDTH - 1) grayscale[iRow][jColume+1] += (char)(7 * error / 16);
					if (iRow < EPD_HEIGHT - 1)
					{
							if (jColume > 0) grayscale[iRow + 1][jColume - 1] += (char)(3 * error / 16);
							grayscale[iRow + 1][jColume] += (char)(5 * error / 16);
							if (jColume < EPD_WIDTH - 1) grayscale[iRow + 1][jColume + 1] += (char)(1 * error / 16);
					}
				}
			}
			//iRow -1 : the row number which already have been converted into 1bpp bitmap image
		}while(iRow < EPD_HEIGHT);
}
