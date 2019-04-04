#ifndef COMMAND_H
#define COMMAND_H

typedef struct __command
{
		bool valid;
		unsigned char name[24];
		unsigned char help[64];
		int (*phandler)(unsigned char command[],unsigned char length);
}cmd;

typedef struct __icon_info{
	bool valid;
	unsigned char xsize;
	unsigned char ysize;
	unsigned char bytes_per_line;
	unsigned char name[20];
	const unsigned char *pointer;
}icon;

/*
* @param cmd buffer and the length of the command
* @param takephoto [u|c] hello.jpg
* @ret return 0 when success
*/
int cmd_takephoto(unsigned char cmd[],unsigned char length);
int cmd_takepicture(unsigned char cmd[],unsigned char length);
int cmd_printer(unsigned char cmd[],unsigned char length);
/*
* @param command buffer and the length of the command string
* @param convert1bpp hello.jpg 0 0 200 200 
* @ret return 0 when success 
*/
int cmd_convert1bpp(unsigned char cmd[],unsigned char length);
/*
* @param command buffer and the length of the command string
* @param convert2bmp [JPG file namee] 
* @ret return 0 when success 
*/
int cmd_convert2bmp(unsigned char cmd[],unsigned char length);
int cmd_convertbimage(unsigned char cmd[],unsigned char length);
/*
* @param browseSDcard
* @output hello.jpg a.bmp c.bmp (four files a row)
* @ret return 0 when success
*/
int cmd_browseSDcard(unsigned char cmd[],unsigned char length);
int cmd_drawrect(unsigned char cmd[],unsigned char length);
int cmd_setfont(unsigned char cmd[],unsigned char length);
int cmd_drawstring(unsigned char cmd[],unsigned char length);
int cmd_drawchar(unsigned char cmd[],unsigned char length);
int cmd_drawdot(unsigned char cmd[],unsigned char length);
int cmd_drawicon(unsigned char cmd[],unsigned char length);
int cmd_drawmenu(unsigned char cmd[],unsigned char length);
int cmd_drawcircle(unsigned char cmd[],unsigned char length);
int cmd_cleardisplay(unsigned char cmd[],unsigned char length);
int cmd_writedisplay(unsigned char cmd[],unsigned char length);
int cmd_helpinfo(unsigned char cmd[],unsigned char length);
int cmd_remove(unsigned char cmd[],unsigned char length);
int cmd_removeall(unsigned char cmd[],unsigned char length);
int cmd_findvalidfilename(unsigned char cmd[],unsigned char length);
int cmd_getbuttonevent(unsigned char cmd[],unsigned char length);
int cmd_display(unsigned char cmd[],unsigned char length);
int cmd_setdismode(unsigned char cmd[],unsigned char length);
int getUserInput(unsigned char input[],unsigned char length);
void callbackRGB(int x, int y, uint8_t* rgb);
void flushRGB(int x, int y, uint8_t* rgb);
void flush1BPP(int x, int y, uint8_t* rgb);
void flushBImage(int x, int y, uint8_t* rgb);
#endif

