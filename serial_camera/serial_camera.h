#ifndef SERIAL_CAMERA_H
#define SERIAL_CAMERA_H

#define RX_USE_INTERRUPT 1

//CJ-OV528 module command code 
#define COMMAND_INIT 0x01
#define COMMAND_GET_PIC 0x04
#define COMMAND_SNAPSHOT 0x05
#define COMMAND_SET_PACKET_SIZE 0x06
#define COMMAND_SET_BAUDRATE 0x07
#define COMMAND_RESET 0x08
#define COMMAND_POWER_DOWN 0x09
#define COMMAND_DATA 0x0a
#define COMMAND_SYNC 0x0d
#define COMMAND_ACK 0x0e
#define COMMAND_NACK 0x0f

// command 01 initial setting
#define COLOR_SETTING_2_GRAY 0x01
#define COLOR_SETTING_4_GRAY 0x02
#define COLOR_SETTING_8_GRAY 0x03
#define COLOR_SETTING_2_COLOR 0x05
#define COLOR_SETTING_16_COLOR 0x06
#define COLOR_SETTING_JPEG 0x07

//command get picture 
#define GET_PIC_SNAPSHOT 0x01
#define GET_PIC_PREVIEW  0x02
#define GET_PIC_JPEG     0x03

#define PREVIEW_RESOLUTION_80_60 0x01
#define PREVIEW_RESOLUTION_160_120 0x03

#define JPEG_RESOLUTION_640_320 0x07
#define JPEG_RESOLUTION_320_240 0x05

/******** Camera Configuration ********/
#define PIC_PACKET_LENGTH 128
#define PIC_FORMAT_VGA 7
#define PIC_FORMAT_CIF 5
#define PIC_FORMAT_OCIF 3

typedef struct __serial_camera_cmd {
		char header;
		char command;
		char param1;
		char param2;
		char param3;
		char param4;
		char name[32];
		int ret;
}serial_camera_cmd;

typedef struct __package_info{
	unsigned short index;		//the index of package , start from 0
	unsigned short size;		//the expected size of the package
	#if RX_USE_INTERRUPT
	char *pBuf;
	#else
	char buffer[512];
	#endif
	unsigned short verify;		//the verify code of current package
}package_info;

typedef struct __picture_info{
	unsigned long size;    //the size of a total picture
	unsigned short width;
	unsigned short height;
	char type;   		   //snapshot , preview , JPEG
	char name[16];   	   //the name of the picture
	char is_compressed;    //is the picture compressed or not
	int (*phandler)(package_info *);	//the handler of a specific package
	package_info *package;
}picture_info;

typedef struct __rx_buffer{
	unsigned short completed;
	unsigned short position; //the position of next byte to receive
	unsigned short number; //the number of current received bytes
	char buffer[512];
}rx_buffer;

void RxIntHandler(void);

void clearRxBuf();
void sendCmd(char command[],int cmd_len);
#if RX_USE_INTERRUPT
int getResponse(char **response,unsigned short length,unsigned short retry);
#else
int getResponse(char response[],unsigned short length,unsigned short retry);
#endif
/*
*@func verifyPackage
*@param response buffer and number of received bytes
*@return -1 when failure
		  0 when success
*/
int verifyPackage(char response[],unsigned short size);
int is_ack(char response[],unsigned short cmd,unsigned short length);
int donothing(package_info *ppackage);

void initialize(void);
void resetCamera();
int cameraSnapshot(picture_info *pInfo);
int cameraGo(picture_info *pInfo);
int makeConnection(char cmd[],unsigned short length);
int instructCamera(char cmd[],unsigned short length,char *cmd_name);
int getPictureData(picture_info *pInfo);
void preCapture();

#endif

