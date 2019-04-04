#include "mbed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "SimpleJpegDecode.h"
#include "bmp24.h"
#include "bmp1.h"
#include "SDFileSystem.h"
#include "serial_camera.h"
#include "EaEPaper.h"
#include "picture.h"
#include "beatiful.h"
#include "button.h"
#include "Thermal.h"
#include "Adafruit_Thermal.h"
#include "font.h"
#include "adafruit.h"
#include "adaqrcode.h"

extern Serial debugcom;
extern rx_buffer receive;
extern EaEpaper epaper;
extern SDFileSystem sd;
SimpleJpegDecode decode;
Adafruit_Thermal printer(P2_0,P2_1,19200);
bmp24 bmp;
bmp1  bpp;

#define MAX_PICTURES 1000
#define FILENAME_FORMAT "%03d.jpg"

cmd cmdpool[30] = 
{
		//abcdefghijklmnopqrstu
		{true,"help","help",cmd_helpinfo},
		{true,"remove","remove",cmd_remove},
		{true,"removeall","removeall",cmd_removeall},
		{true,"findvalidfilename","findvalidfilename",cmd_findvalidfilename},
		{true,"takephoto","takephoto 640 480 /sd/PIC100.JPG",cmd_takephoto},
		{true,"takepicture","takepicture /sd/PIC200.JPG",cmd_takepicture},
		{true,"printer","printer command",cmd_printer},
		{true,"convert1bpp","convert1bpp 320 240 [JPEG] [BMP]",cmd_convert1bpp},
		{true,"convert2bmp","convert2bmp 640 480 [JPEG] [BMP]",cmd_convert2bmp},
		{true,"convertbbmp","convertbbmp 640 480 128 [JPEG] [BMP]",cmd_convertbimage},
		{true,"ls","browseSDcard",cmd_browseSDcard},
		{true,"drawrect","drawrect 0 0 20 20",cmd_drawrect},
		{true,"setfont","setfont [default|font]",cmd_setfont},
		{true,"drawstring","drawstring 0 0 hello",cmd_drawstring},
		{true,"drawchar","drawchar 0 0 A",cmd_drawchar},
		{true,"drawdot","drawdot 0 0",cmd_drawdot},
		{true,"drawicon","drawicon 0 0 hello.bmp",cmd_drawicon},
		{true,"drawmenu","drawmenu [index | 0-8] filename",cmd_drawmenu},
		{true,"getbuttonevent","getbuttonevent",cmd_getbuttonevent},
		{true,"display","display <menu ID> <item ID>",cmd_display},
		{true,"clear","clear",cmd_cleardisplay},
		{true,"setdismode","setdismode [normal | xor]",cmd_setdismode},
		{true,"drawcircle","drawcicle x y",cmd_drawcircle},
		{true,"writedisplay","writedisplay",cmd_writedisplay},
		0,
};

cmd printercmds[10] = {
	{true,"help","show printer command help info",NULL},
	{true,"test","print hello world",NULL},
	{true,"barcode","print barcode",NULL},
	{true,"fancybarcode","print fancy barcode",NULL},
	NULL,
};

icon iconpool[10] = 
{
	{true,24,24,3,"buttonA",buttonA},
	{true,24,24,3,"buttonGo",buttonGo},
	{true,200,176,25,"camerabag",camera_bag},
	{true,40,120,5,"buttonTakePhoto",buttonTakePhoto},
	0,
};

int cmd_helpinfo(unsigned char cmd[],unsigned char length)
{
		unsigned short i = 0;
		for(i=0;i<sizeof(cmdpool)/sizeof(cmdpool[0]) && cmdpool[i].valid == true ;i++)
				debugcom.printf("%-20s %s \n\r",cmdpool[i].name,cmdpool[i].help);
		return 0;
}

int cmd_remove(unsigned char cmd[],unsigned char length)
{
	char *pfile = NULL;
	strtok((char *)cmd," ");
	pfile = strtok(NULL," ");
	if(remove(pfile) == 0)

	{
		debugcom.printf("%s %s %d %s removed .\r\n",__FILE__,__FUNCTION__,__LINE__,pfile);
		return 0;
	}
	else
	{
		debugcom.printf("%s %s %d %s remove failed .\r\n",__FILE__,__FUNCTION__,__LINE__,pfile);
		return -1;
	}
}

/***************************************** 
* @func cmd_removeall 
* @param cmd buffer
* @usage removeall 
* @return 
*****************************************/
int cmd_removeall(unsigned char cmd[],unsigned char length)
{
	char filename[30] = {0};
	unsigned char i = 0;
	
	for(i=0;i<MAX_PICTURES;i++)
	{
		sprintf(filename,"remove /sd/%03d.JPG",i);
		cmd_remove((unsigned char *)filename,sizeof(filename));
		sprintf(filename,"remove /sd/%03d.BMP",i);
		cmd_remove((unsigned char *)filename,sizeof(filename));
	}
	return 0;
}

/***************************************** 
* @func cmd_findvalidfilename 
* @param cmd buffer
* @usage findvalidfilename 
* @return a positive interger [0-99] when success
	return -1 when all file name used
	return -2 when open SD card failed
*****************************************/
int isJPGfile(unsigned char *filetype)
{
	if(strcmp((char *)filetype,"jpg") == 0 || strcmp((char *)filetype,"JPG") == 0)
		return true;
	else
		return false;
}

int cmd_findvalidfilename(unsigned char cmd[],unsigned char length)
{
	unsigned short index = 0;
	unsigned short i = 0;
	unsigned char file_existed[MAX_PICTURES] = {0}; 		//flags indicate which picture existed
	char *pfiletype = NULL;
	
	debugcom.printf("%s %s %d %s \n\r",__FILE__,__FUNCTION__,__LINE__,cmd);

	struct dirent *ptr = NULL;
	DIR *pDir = opendir("/sd");
	if(pDir == NULL){
		debugcom.printf("Open Directory /sd failed ./r/n");
		return -2;
	}
	while((ptr = readdir(pDir)) != NULL){
		index = (unsigned short)strtoul(strtok(ptr->d_name,"."),NULL,10);		//use default value
		pfiletype = strtok(NULL,".");
		debugcom.printf("%s %s %d %03d%s \r\n",__FILE__,__FUNCTION__,__LINE__,index,pfiletype);
		//FILE name starts from 0 , ends at 99
		if(index <= MAX_PICTURES-1 && isJPGfile((unsigned char *)pfiletype))
			file_existed[index] = true;
	}
	closedir(pDir);

	for(i=0;i<MAX_PICTURES;i++)
		if(file_existed[i] != true)
			return i;
	return -1;
	
}

/* 
* @func cmd_takephoto
* @param cmd buffer
* @usage takephoto [u|c] hello.jpg
* @return 0 when success
	return -1 when invalid parameter
*/
int cmd_takephoto(unsigned char cmd[],unsigned char length)
{
		debugcom.printf("%s %s %d %s \n\r",__FILE__,__FUNCTION__,__LINE__,cmd);

		package_info package;
		picture_info picture;
		
		strtok((char *)cmd," ");

		package.index = 0;
		package.size = 512;
		package.pBuf = receive.buffer;

		picture.width = (unsigned short)strtoul(strtok(NULL," "),NULL,10);		//use default value
		picture.height = (unsigned short)strtoul(strtok(NULL," "),NULL,10);		//use default value
		picture.type = GET_PIC_JPEG;//use default value
		strcpy(picture.name,strtok(NULL," ")); //use user specific file name
		picture.package = &package;
		picture.phandler = donothing;
		picture.is_compressed = true;
		
	  	debugcom.printf("Initialize Camera ................................. \n\r");
		initialize();

    	debugcom.printf("Reset Camera ...... \n\r");
		resetCamera(); //maybe different from previous camera
		if (0 == cameraSnapshot(&picture))
			debugcom.printf("Camera Snap Shot Done ......\r\n");
		else
			debugcom.printf("Camera Snap Shot Failed .......\r\n");
		return 0;
}

/* 
* @func cmd_takepicture
* @param cmd buffer
* @usage takepicture hello.jpg
* @return 0 when success
	return -1 when failure 
*/
int cmd_takepicture(unsigned char cmd[],unsigned char length)
{
		debugcom.printf("%s %s %d %s \n\r",__FILE__,__FUNCTION__,__LINE__,cmd);

		package_info package;
		picture_info picture;
		
		strtok((char *)cmd," ");

		package.index = 0;
		package.size = 512;
		package.pBuf = receive.buffer;

		picture.width = 320;		//use default value
		picture.height = 240;		//use default value
		picture.type = 0x05;		//use default value
		strcpy(picture.name,strtok(NULL," ")); //use user specific file name
		picture.package = &package;
		picture.phandler = donothing;
		picture.is_compressed = true;
		
	  	debugcom.printf("Initialize Camera ................................. \n\r");
		initialize();

		if (0 == cameraGo(&picture)){
			debugcom.printf("Camera Snap Shot Done ......\r\n");
			return 0;
		}
		else{
			debugcom.printf("Camera Snap Shot Failed .......\r\n");
			return -1;
		}
}

/************************************
*@func all printer relative commands
*@param
*@ret return -1 when invalid command
************************************/
int cmd_printer(unsigned char cmd[],unsigned char length)
{
	debugcom.printf("%s %s %d %s \n\r",__FILE__,__FUNCTION__,__LINE__,cmd);
	strtok((char *)cmd," ");
	char *p_Cmd = strtok(NULL," ");	//printer sub command name

	printer.begin();

	if(strcmp(p_Cmd,"help") == 0)
	{
	}
	else if(strcmp(p_Cmd,"test") == 0)
	{
		printer.test();
	}
	else if(strcmp(p_Cmd,"inverseon") == 0)
	{
		printer.inverseOn();
		printer.printf("Inverse ON");
		printer.feed(2);
		printer.inverseOff();
	}
	else if(strcmp(p_Cmd,"boldtext") == 0)
	{
		// Test more styles
		printer.boldOn();
		printer.printf("Bold text");
		printer.feed(2);
		printer.boldOff();
	}
	else if(strcmp(p_Cmd,"adafruit") == 0)
	{
		// Print the 75x75 pixel logo in adalogo.h
		printer.printBitmap(adalogo_width, adalogo_height, adalogo_data,false);
	}
	else if(strcmp(p_Cmd,"qrcode") == 0)
	{
		// Print the 75x75 pixel logo in adalogo.h
		printer.printBitmap(adaqrcode_width, adaqrcode_height, adaqrcode_data,false);
		printer.printf("Adafruit!");
		printer.feed(1);
	}
	else if(strcmp(p_Cmd,"bag") == 0)
	{
		// Print the 75x75 pixel logo in adalogo.h
		printer.printBitmap(200, 176, camera_bag,false);
		printer.printf("Adafruit!");
		printer.feed(1);
	}
	else if(strcmp(p_Cmd,"bitmap") == 0)
	{
		char *pinverse = strtok(NULL," ");
		if(strcmp(pinverse,"invert") == 0)
			printer.printBitmap(320, 240, &bpp.m_bitmap[0][0],true);
		else
			printer.printBitmap(320, 240, &bpp.m_bitmap[0][0],false);
	}
	else if(strcmp(p_Cmd,"barcode") == 0)
	{
	  // Barcode examples
	  printer.feed(1);
	  // CODE39 is the most common alphanumeric barcode
	  printer.printBarcode("ADAFRUT", CODE39);
	  printer.setBarcodeHeight(100);
	  // Print UPC line on product barcodes
	  printer.printBarcode("123456789123", UPC_A);
	}
	else{
		debugcom.printf("%s %s %d invalid printer command \r\n",
			__FILE__,__FUNCTION__,__LINE__);
		return -1;
	}
	return 0;
}

int cmd_convertbimage(unsigned char cmd[],unsigned char length)
{
	debugcom.printf("%s %s %d %s \n\r",__FILE__,__FUNCTION__,__LINE__,cmd);
	strtok((char *)cmd," ");
	unsigned short w = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	unsigned short h = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	unsigned short b = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	char *pJpgfile = strtok(NULL," ");	//input file name
	char *pBmpfile = strtok(NULL," ");  //output file name

	bpp.resizeImage(w,h);
	bpp.setbalance(b);
	bpp.clear();
	
	FILE* fp = fopen(pJpgfile, "r+");
	if (fp == NULL) {
		 debugcom.printf("Open JPEG file %s failed \r\n",pJpgfile);
		 return -1;
	}
    decode.setOnResult(flushBImage);
    decode.clear();
    while(!feof(fp)) {
        int c = fgetc(fp);
        decode.input(c);
    }
    fclose(fp);

	bpp.writeFile(pBmpfile);
	return 0;
}

int cmd_convert1bpp(unsigned char cmd[],unsigned char length)
{
	debugcom.printf("%s %s %d %s \n\r",__FILE__,__FUNCTION__,__LINE__,cmd);
	strtok((char *)cmd," ");
	unsigned short w = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	unsigned short h = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	char *pJpgfile = strtok(NULL," ");	//input file name
	char *pBmpfile = strtok(NULL," ");  //output file name

	bpp.resizeImage(w,h);
	bpp.clear();
	
	FILE* fp = fopen(pJpgfile, "r+");
	if (fp == NULL) {
		 debugcom.printf("Open JPEG file %s failed \r\n",pJpgfile);
		 return -1;
	}
    decode.setOnResult(flush1BPP);
    decode.clear();
    while(!feof(fp)) {
        int c = fgetc(fp);
        decode.input(c);
    }
    fclose(fp);

	bpp.writeFile(pBmpfile);
	return 0;
}

int cmd_convert2bmp(unsigned char cmd[],unsigned char length)
{
	debugcom.printf("%s %s %d %s \n\r",__FILE__,__FUNCTION__,__LINE__,cmd);
	strtok((char *)cmd," ");
	unsigned short w = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	unsigned short h = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	char *pJpgfile = strtok(NULL," ");	//input file name
	char *pBmpfile = strtok(NULL," ");  //output file name
	FILE* fp = fopen(pJpgfile, "r+");
	if (fp == NULL) {
		 debugcom.printf("Open JPEG file %s failed \r\n",pJpgfile);
		 return -1;
	}

    if(false == bmp.openFile(pBmpfile))
		debugcom.printf("%s %s %d open file failed ",__FILE__,__FUNCTION__,__LINE__,pBmpfile);
	bmp.initFile(w,h);
	
    decode.setOnResult(flushRGB);
    decode.clear();
    while(!feof(fp)) {
        int c = fgetc(fp);
        decode.input(c);
    }
    fclose(fp);

	bmp.closeFile();
	return 0;
}

int cmd_browseSDcard(unsigned char cmd[],unsigned char length)
{
	unsigned short i = 0;
	
	debugcom.printf("%s %s %d %s \n\r",__FILE__,__FUNCTION__,__LINE__,cmd);

	struct dirent *ptr = NULL;
	DIR *pDir = opendir("/sd");
	if(pDir == NULL){
		debugcom.printf("Open Directory /sd failed ./r/n");
		return -1;
	}
	
	while((ptr = readdir(pDir)) != NULL){

		i++;
		debugcom.printf("		%s",ptr->d_name);
		if(i%5 == 0)
			debugcom.printf("\r\n");
	}
	debugcom.printf("\r\n");
	closedir(pDir);
	
	return 0;
}

int cmd_drawrect(unsigned char cmd[],unsigned char length)
{
		return 0;
}

int cmd_setfont(unsigned char cmd[],unsigned char length)
{

	epaper.set_font((unsigned char *)Consolas10x22);
	return 0;
}

/*********************
* @func drawstring 
* @para input buffer and the size of the buffer
* @usage drawstring 0 0 hello.jpg
* @return 0 when success
     return -1 when null string
*********************/
int cmd_drawstring(unsigned char cmd[],unsigned char length)
{
	debugcom.printf("%s %s %d %s \n\r",__FILE__,__FUNCTION__,__LINE__,cmd);

	strtok((char *)cmd," ");
	unsigned char x = (unsigned char)strtoul(strtok(NULL," "),NULL,10);
	unsigned char y = (unsigned char)strtoul(strtok(NULL," "),NULL,10);
	unsigned char str[32] = {0}; 	
	int ret = 0;
	
	strcpy((char *)str,strtok(NULL," "));

	ret = epaper.print_string((char *)str, x, y);
	debugcom.printf("ret = %d \r\n",ret);
	
	return 0;
}
/*********************
* @func drawchar
* @para input buffer and the size of the buffer
* @usage drawchar 0 0 c
* @return 0 when success
     return -1 when null string
*********************/
int cmd_drawchar(unsigned char cmd[],unsigned char length)
{
	strtok((char *)cmd," ");
	unsigned short x = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	unsigned short y = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	char *pchar = strtok(NULL," ");
	debugcom.printf("%s %s %d %d %d %c \n\r",__FILE__,__FUNCTION__,__LINE__,x,y,*pchar);
	int ret_val = epaper.putchar((int)x,(int)y,*pchar);
	debugcom.printf("%d \r\n",ret_val);
	return 0;
}

int cmd_drawdot(unsigned char cmd[],unsigned char length)
{
	strtok((char *)cmd," ");
	unsigned short x = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	unsigned short y = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	epaper.pixel((int)x, (int)y, 1);
	return 0;
}
/*********************
* @func set display mode  
* @para input buffer and the size of the buffer
* @usage setdismode [normal : 0 | xor : 1]
* @return 0 when success
     return -1 when null string
*********************/
int cmd_setdismode(unsigned char cmd [ ], unsigned char length)
{
	debugcom.printf("%s %s %d %s \n\r",__FILE__,__FUNCTION__,__LINE__,cmd);
	strtok((char *)cmd," ");
	unsigned char mode = (unsigned char)strtoul(strtok(NULL," "),NULL,10);
	//current mode : NORMAL XOR
	epaper.setmode((int) mode);
	return 0;
}

int cmd_cleardisplay(unsigned char cmd[],unsigned char length)
{
	debugcom.printf("%s %s %d %s \n\r",__FILE__,__FUNCTION__,__LINE__,cmd);
	epaper.clear();
	epaper.write_disp();
	return 0;
}

int cmd_writedisplay(unsigned char cmd[],unsigned char length)
{
	epaper.write_disp();
	return 0;
}


/*********************
* @func drawdot  
* @para input buffer and the size of the buffer
* @usage drawcircle <x> <y> 
* @return 0 when success
     return -1 when null string
*********************/
int cmd_drawcircle(unsigned char cmd[],unsigned char length)
{
	debugcom.printf("%s %s %d %s \n\r",__FILE__,__FUNCTION__,__LINE__,cmd);
	strtok((char *)cmd," ");
	unsigned short x = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	unsigned short y = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	if(x > 275 || y > 176)
		debugcom.printf("%s %s %d Invalid parameter \n\r",__FILE__,__FUNCTION__,__LINE__);
	else
		debugcom.printf("%s %s %d %d %d \n\r",__FILE__,__FUNCTION__,__LINE__,x,y);
	epaper.fillcircle(x,y,5,1);
	return 0;
}
/*********************
* @func drawimage  
* @para input buffer and the size of the buffer
* @usage drawicon x y imagename
* @return 0 when success
     return -1 when null string
*********************/
int cmd_drawicon(unsigned char cmd[],unsigned char length)
{
	debugcom.printf("%s %s %d %s \n\r",__FILE__,__FUNCTION__,__LINE__,cmd);
	strtok((char *)cmd," ");
	unsigned short x = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	unsigned short y = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	char *pimage = strtok(NULL," ");
	struct Bitmap bmpinfo;
	bool found = false;

	for(int i = 0;i<sizeof(iconpool)/sizeof(iconpool[0]) && iconpool[i].valid == true ; i++)
	{
		if(strcmp(pimage,(char *)iconpool[i].name) == 0){
			bmpinfo.xSize = iconpool[i].xsize;
			bmpinfo.ySize = iconpool[i].ysize;
			bmpinfo.Byte_in_Line = iconpool[i].bytes_per_line;
			bmpinfo.data = (char *)iconpool[i].pointer;
			found = true;
			break;
		}
	}

	if(!found){
		debugcom.printf("Invalid icon name .\r\n");
		return -1;
	}
			
	debugcom.printf("Drawing ICON %s ...... \r\n",pimage);
	epaper.print_bm(bmpinfo,x,y);
	
	return 0;
}

/*********************
* @func drawmenu  
* @para input buffer and the size of the buffer
* @usage drawmenu [index | 0-8] [filename] 
* @return 0 when success
     return -1 when null string
*********************/
int cmd_drawmenu(unsigned char cmd[],unsigned char length)
{
	debugcom.printf("%s %s %d %s \n\r",__FILE__,__FUNCTION__,__LINE__,cmd);
	strtok((char *)cmd," ");
	unsigned short index = (unsigned short)strtoul(strtok(NULL," "),NULL,10);
	char *pfilename = strtok(NULL," ");
	struct Bitmap bmpinfo;
	
	bmpinfo.xSize = 24;
	bmpinfo.ySize = 24;
	bmpinfo.Byte_in_Line = 3;
	bmpinfo.data = (char *)buttonGo;

	epaper.print_string(pfilename, index*24, 175);
	epaper.line((index+1)*24-1, 175, (index+1)*24-1, 25, 1);
	epaper.print_bm(bmpinfo, index*24, 0);
	return 0;
}



int cmd_getbuttonevent(unsigned char cmd [ ], unsigned char length)
{
	return 0;
}

/***************************
*@func refresh display
*@param input comand buffer and the size of the command
*@usage display <menuID> <ItemID>
***************************/
int cmd_display(unsigned char cmd [ ], unsigned char length)
{	
	return 0;
}

/*
* @param input buffer and the length of the buffer
* @ret number of chars received
* @special '\b' '\n'
*/
int getUserInput(unsigned char input[],unsigned char length)
{
		unsigned short i=0;
		do
		{
				input[i] = (unsigned char)debugcom.getc();
				debugcom.putc((int)input[i]);
				if(input[i] == '\b' && i > 0)
							i--;
				else if(input[i] == '\r')
						break;
				else
						i++;
				// i the next available location
		}while(i<length-1);
		input[i] = '\0';
		return i;
}

void callbackRGB(int x, int y, uint8_t* rgb)
{
    bmp.point(x, y, rgb);
}

void flushRGB(int x, int y, uint8_t* rgb)
{
	bmp.flushblock(x,y,rgb);
}

void flush1BPP(int x, int y, uint8_t* rgb)
{
	bpp.flushpoint(x,y,rgb);
}

void flushBImage(int x, int y, uint8_t* rgb)
{
	bpp.point(x,y,rgb);
}
