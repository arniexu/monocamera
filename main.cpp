#include "mbed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDFileSystem.h"
#include <SPI.h>

#include "serial_camera.h"
#include "EaEPaper.h"
#include "joystick.h"
#include "debug.h"
#include "command.h"

#define ENABLE_DEBUG 0

/********* HW Configuration **********/
SDFileSystem sd(P0_18,P0_17,P0_15,D4,"sd");
EaEpaper epaper(P0_4,P0_5,P0_0,P2_4,P2_5,P0_6,P2_3,P0_18,P0_17,P0_15,D14,D15);
//			 epaper(D2,  D3,  D8,  D6,  D7,  D10, D5,  SPI BUS,          D14,D15);
Serial debugcom(USBTX,USBRX);
/*  debugcom pin mapping relationship 
		USBTX P0_2
		USBRX P0_3
*/	

#define ENABLE_CMDLINE 0

DigitalOut PhotoLED(LED1);
DigitalOut ConvertLED(LED2);
DigitalOut PrinterLED(LED3);
DigitalOut FinalLED(LED4);
DigitalIn Button(P0_27);
DigitalOut OutIndicator(P2_5);


extern cmd cmdpool[20];
extern _Joystick input;

#define IN_CMDLINE 0
#define EXIT_CMDLINE 0
#define WAIT_BUTTON 0x05
#define TAKEPHOTO_DONE 1
#define CONVERT_DONE 2
#define PRINTER_DONE 4

void turnonLED(int status,int blink){
	if(status >= 0 && status <= 15)
	{
		if(status & 0x01)
			PhotoLED = 0;
		else
			PhotoLED = 1;
		
		if(status & 0x02)
			ConvertLED = 0;
		else
			ConvertLED = 1;
		
		if(status & 0x04)
			PrinterLED = 0;
		else
			PrinterLED = 1;
		
		if(status & 0x08)
			FinalLED = 0;
		else
			FinalLED = 1;
	}
	while(blink-- > 0)
	{
		OutIndicator = 1;
		wait_ms(60);
		OutIndicator = 0;
		wait_ms(60);
	}
}

int main()
{

	char execcmds[5][64] = {
		{"remove /sd/pic001.jpg "},
		{"remove /sd/bmp001.bmp "},
		{"takepicture /sd/pic001.jpg "},
		{"convertbbmp 320 240 128 /sd/pic001.jpg /sd/bmp001.bmp "},
		{"printer bitmap dontinvert "},
	};
	int ret = -2;

	debugcom.baud(38400);
	
	debugcom.printf("Initialize JOYSTICK ................\r\n");
	initialize_joystick();
	
	turnonLED(EXIT_CMDLINE,0);
	
	while(1)
	{
		debugcom.printf("Right Button pressed .\r\n");
		turnonLED(0,0);
		sprintf(execcmds[0],"%s ","findvalidfilename");
		sprintf(execcmds[1],"%s ","removeall");
		
		ret = cmd_findvalidfilename((unsigned char *)execcmds[0],64);
		if(ret>=0)
			;
		else if(ret == -2)
			continue;	//goto waiting button state
		else{
			cmd_removeall((unsigned char *)execcmds[1],64); //delete all pictures
			ret = 0; //start from 0
		}
		
		debugcom.printf("%s %s %d %d \r\n",__FILE__,__FUNCTION__,__LINE__,ret);
		
		sprintf(execcmds[2],"takepicture /sd/%03d.jpg",ret);
		sprintf(execcmds[3],"convertbbpp 320 240 128 /sd/%03d.jpg /sd/%03d.bmp",ret,ret);
		sprintf(execcmds[4],"%s ","printer bitmap dontinvert");
		
		if(0 == cmd_takepicture((unsigned char *)execcmds[2], sizeof(execcmds[2]))) 
		{
			turnonLED(TAKEPHOTO_DONE,2);
			if(0 == cmd_convertbimage((unsigned char *)execcmds[3], sizeof(execcmds[3])))
			{
				turnonLED(CONVERT_DONE|TAKEPHOTO_DONE,2);
				if(0 == cmd_printer((unsigned char *)execcmds[4], sizeof(execcmds[4])))
					turnonLED(PRINTER_DONE|CONVERT_DONE|TAKEPHOTO_DONE,2);
			}
		}
		wait_ms(60*1000);
		turnonLED(WAIT_BUTTON,1);	
		wait_ms(125);
		turnonLED(0x0a,1);
		wait_ms(125);
	}
}

