#ifndef BMP24_H
#define BMP24_H

#define BMP24_WIDTH  (320)
#define BMP24_HEIGHT (8)
extern Serial debugcom;

class bmp24 {
    uint8_t m_bitmap[BMP24_WIDTH*BMP24_HEIGHT*3];
    int width;
    int height;
	FILE *fd;
	uint8_t header[54];
public:
    bmp24() {
        width = BMP24_WIDTH;
        height = BMP24_HEIGHT;
		fd = NULL;
		memset(header,0,sizeof(header));
		header[0] = 0x42;
		header[1] = 0x4d;	//bitmap file type
		
		header[2] = 0x36;	//bitmap file size
		header[3] = 0xe1;
			
		header[10] = 0x36;

		header[14] = 0x28; //data structure size

		header[18] = 0xa0;
		
		header[22] = 0x78;

		header[26] = 0x01;
		header[28] = 0x18;

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
			0x00,0x00,0x00,0x00,0x00,0x00,0x00, //number of pixel per meter horizonal
			0x00,0x00,0x00,0x00,0x00,0x00,0x00, 	//number of pixel per meter verticle 
			0x00,	//the actually used colors in color table
			0x00,	//important colors in color table
			};
			*/
    }
    
    void clear() {
        memset(m_bitmap, 0, sizeof(m_bitmap));
    }

    void point(int x, int y, uint8_t* rgb) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            int pos = y*width*3 + x*3;			
            m_bitmap[pos++] = rgb[0];
            m_bitmap[pos++] = rgb[1];
            m_bitmap[pos]   = rgb[2];
        }
    }

	void flushpoint(int x, int y, uint8_t *rgb){
		debugcom.printf("%d %d \r\n",x,y);
        if (x >= 0 && x < width && y >= 0 && y < height) {
			long offset = (height-y-1)*width*3 + x*3 + sizeof(header);
			fseek(fd,offset,SEEK_SET);
			fputc(rgb[2],fd);
			fputc(rgb[1],fd);
			fputc(rgb[0],fd);
        }
	}

	void flushblock(int x,int y,uint8_t *rgb){
		int pos = 0;
		
        if (x >= 0 && x < width && y >= 0 && y < height) {

			pos = (x%BMP24_WIDTH)*3 + (y%BMP24_HEIGHT)*BMP24_WIDTH*3;

            m_bitmap[pos++] = rgb[2];
            m_bitmap[pos++] = rgb[1];
            m_bitmap[pos]   = rgb[0];
			
			if(x%BMP24_WIDTH == BMP24_WIDTH-1 && y%BMP24_HEIGHT == BMP24_HEIGHT-1)
				writeblock(x-BMP24_WIDTH+1,y-BMP24_HEIGHT+1);
		}
    }
    
    void LE32write(uint8_t* buf, int value) {
        *buf++ = value & 0xff;
        *buf++ = (value>>8) & 0xff;
        *buf++ = (value>>16) & 0xff;
        *buf   = (value>>24) & 0xff;
    }

	
    bool openFile(const char *path) {
		fd = fopen(path,"w+");
		if(fd == NULL)
			return false;
		else
			return true;
    }

	void initFile(int w ,int h) {
		width = w;
		height = h;
        LE32write(header+2, sizeof(header) + width*height*3);
        LE32write(header+18, width);
        LE32write(header+22, height);
        fwrite(header, 1, sizeof(header), fd);
	}

	bool closeFile(void) {
		return fclose(fd);
	}

	bool writeblock(int x,int y) {
		long offset = sizeof(header) + (height-y+1)*width*3 + x*3;
		int i = 0;

		for(i=0;i<BMP24_HEIGHT;i++){
			fseek(fd,offset-width*3*i,SEEK_SET);
			fwrite(&m_bitmap[BMP24_WIDTH*3*i],1,BMP24_WIDTH*3,fd);
		}
		
		debugcom.printf("%s %s %d %d %d \r\n",__FILE__,__FUNCTION__,__LINE__,x,y);
		return 0;
	}
	
    bool writeFile(const char *path) {
        FILE *fp = fopen(path, "wb");
        if (fp == NULL) {
            return false;
        }
		
        int file_size = sizeof(header) + sizeof(m_bitmap);
        LE32write(header+2, file_size);
        LE32write(header+18, width);
        LE32write(header+22, height);
        
        fwrite(header, 1, sizeof(header), fp);
        for(int y = height-1; y >=0; y--) {
            for(int x = 0; x < width; x++) {
                fputc(m_bitmap[y*width*3+x*3+2], fp);
                fputc(m_bitmap[y*width*3+x*3+1], fp);
                fputc(m_bitmap[y*width*3+x*3+0], fp);
            }
        }    
        fclose(fp);
        return true;
    }
};

#endif // BMP24_H
