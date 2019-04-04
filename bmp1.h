#ifndef BMP1_H
#define BMP1_H

#define BMP1_WIDTH  (320)
#define BMP1_HEIGHT (240)

#define GRAYBLOCK_HEIGHT (8)

#define COLORFORMAT_1BPP 1
#define COLORS_IN_COLORPALLETE 2

extern Serial debugcom;

/****************************
 *@ func convert Jpeg data into 1bpp format bmp data
 *@	specific for 320X240 image
 *@
****************************/

class bmp1 {
	signed char m_grayscale[GRAYBLOCK_HEIGHT+1][BMP1_WIDTH];
    int width;
    int height;
	uint8_t header[54];
	uint8_t colorpallete[8];
	uint8_t balance;
	
	double getGrayLevel(unsigned char r,unsigned char g,unsigned char b)
	{
		return (r * 0.599 + g * 0.287 + b * 0.114) / 255.0;
	}
	
	/***************************
	*@ func convert first 7 row of grayscale data into 1bpp format and save the 8th row
	*@ param (x,y) Absolute coordinates within an image , start position of current row
	*@ param rows2convert number of rows to convert 
	*@ precondition [0-7] [8-15] [16-23] transform like this 
	*@ return 0
	***************************/
	void ConvertTo1Bpp(int x,int y,int rows2convert)
	{
		unsigned char masks[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
		unsigned short iRow = 0;  // the ith row piexels of the JPEG image
		unsigned short jColume = 0;  // the jth pixel in i row of the JPEG image

		for(iRow = 0; iRow < rows2convert; iRow ++)
		{
			for(jColume = x ; jColume < x+BMP1_WIDTH ; jColume ++)
			{
				signed char error;
				if(m_grayscale[iRow][jColume] > 0)
						m_bitmap[iRow+y][jColume/8] |= masks[jColume%8];
				error = m_grayscale[iRow][jColume] - (m_grayscale[iRow][jColume] > 0 ? 32 : -32);
				if (jColume < BMP1_WIDTH - 1) m_grayscale[iRow][jColume+1] += (signed char)(7 * error / 16);
				if (iRow+y < BMP1_HEIGHT - 1)
				{
					if (jColume > 0) m_grayscale[iRow + 1][jColume - 1] += (signed char)(3 * error / 16);
					m_grayscale[iRow + 1][jColume] += (signed char)(5 * error / 16);
					if (jColume < BMP1_WIDTH - 1) m_grayscale[iRow + 1][jColume + 1] += (signed char)(1 * error / 16);
				}
			}
		}
	}
public:
    uint8_t m_bitmap[BMP1_HEIGHT][BMP1_WIDTH/8];
    bmp1() {
        width = BMP1_WIDTH;
        height = BMP1_HEIGHT;
		memset(&m_bitmap[0][0],0,sizeof(m_bitmap));
		memset(&m_grayscale[0][0],0,sizeof(m_grayscale));
		memset(&header[0],0,sizeof(header));
		memset(&colorpallete[0],0,sizeof(colorpallete));
		balance = 128;
		header[0] = 0x42;
		header[1] = 0x4d;	//bitmap file type
		
		header[2] = 0x36;	//bitmap file size
		header[3] = 0xe1;
			
		header[10] = sizeof(header) + sizeof(colorpallete);

		header[14] = 0x28; //data structure size

		header[18] = 0xa0;
		
		header[22] = 0x78;

		header[26] = 0x01;
		header[28] = COLORFORMAT_1BPP;
		header[46] = COLORS_IN_COLORPALLETE;
		header[50] = COLORS_IN_COLORPALLETE;

		colorpallete[0] = 0;
		colorpallete[1] = 0;
		colorpallete[2] = 0;
		colorpallete[3] = 0;	//white index 0

		colorpallete[4] = 0xff;
		colorpallete[5] = 0xff;	
		colorpallete[6] = 0xff;
		colorpallete[7] = 0;	//black index 1
		/*
		uint8_t header[] = {
			0x42,0x4d,	//bitmap file type
			0x36,0xe1,0x00,0x00,	//bitmap file size
			0x00,0x00,0x00,0x00,	//reserved must be zero
			0x36,0x00,0x00,0x00,	//bitmap data start position
		
			0x28,0x00,0x00,0x00,	//this data structure size 40 
			0xa0,0x00,0x00,0x00,	//the width of current bitmap image
			0x78,0x00,0x00,0x00,	//the height of current bitmap image
			0x01,0x00,	//target device rating , must be 1
			0x18,0x00,	//bits for each pixel ( 1/4/8/24 )
			0x00,0x00,0x00,0x00,	//compression type ( 0/1/2 )
			0x00,0x00,0x00,0x00,	//the size of current bitmap image
			0x00,0x00,0x00,0x00,	//number of pixel per meter horizonal
			0x00,0x00,0x00,0x00,	//number of pixel per meter verticle 
			0x00,0x00,0x00,0x00,	//number of used colors in color pallete
			0x00,0x00,0x00,0x00,	//important colors in color table , all important when zero
			};
			*/
    }

	void clear(void){
		memset(&m_bitmap[0][0],0,sizeof(m_bitmap));
		memset(&m_grayscale[0][0],0,sizeof(m_grayscale));
	}
	
	//once we can convert 320X5 pixels
    void flushpoint(int x, int y, uint8_t* rgb) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
			if(y <= 7)
	            m_grayscale[y][x] = (signed char)((getGrayLevel(rgb[0],rgb[1],rgb[2])-0.5)*64);
			else
	            m_grayscale[y%GRAYBLOCK_HEIGHT + 1][x] = (signed char)((getGrayLevel(rgb[0],rgb[1],rgb[2])-0.5)*64);
			
			if( x%BMP1_WIDTH == BMP1_WIDTH-1 && y%GRAYBLOCK_HEIGHT == GRAYBLOCK_HEIGHT-1) 
			{
				debugcom.printf("%d %d \r\n",x,y);
				if(y==7)
				{
					ConvertTo1Bpp(x-BMP1_WIDTH+1,y-7,7); 
					memcpy(m_grayscale[0],m_grayscale[7],BMP1_WIDTH);
				}
				else if(y != 239)
				{
					ConvertTo1Bpp(x-BMP1_WIDTH+1,y-8,8); 
					memcpy(m_grayscale[0],m_grayscale[8],BMP1_WIDTH);
				}
				else
					ConvertTo1Bpp(x-BMP1_WIDTH+1,y-8,9); 
			}
        }
    }
	
    void point(int x, int y, uint8_t* rgb) {
		uint8_t grayscale = rgb[0] * 0.599 + rgb[1] * 0.287 + rgb[2] * 0.114;
		unsigned char masks[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
		
		if(grayscale >= balance)
			m_bitmap[y][x/8] |= masks[x%8];
		
    }
    	
    void LE32write(uint8_t* buf, int value) {
        *buf++ = value & 0xff;
        *buf++ = (value>>8) & 0xff;
        *buf++ = (value>>16) & 0xff;
        *buf   = (value>>24) & 0xff;
    }

	void resizeImage(int w ,int h) {
		width = w;
		height = h;
	}
	
	void setbalance(int b) {
		balance = b;
	}
	
    bool writeFile(const char *path) {
        FILE *fp = fopen(path, "wb");
        if (fp == NULL) {
            return false;
        }
		
        int file_size = sizeof(header) + sizeof(colorpallete) + sizeof(m_bitmap);
        LE32write(header+2, file_size);
        LE32write(header+18, width);
        LE32write(header+22, height);
        
        fwrite(header, 1, sizeof(header), fp);
		fwrite(colorpallete,1,sizeof(colorpallete),fp);
        for(int y = height-1; y >=0; y--) {
			fwrite(m_bitmap[y],1,sizeof(m_bitmap[0]),fp);
        }    
        fclose(fp);
        return true;
    }
	
};

#endif // BMP24_H
