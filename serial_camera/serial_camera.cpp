#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial_camera.h"
#include "mbed.h"
#include "debug.h"
#include "SDFileSystem.h"
#include "EaEPaper.h"

extern SDFileSystem sd;
rx_buffer receive;

/****************** Hardware Configuration ************************/
DigitalOut OE(D9);

/****************** Definition ******************/
// cmd means the packet MCU sends to CAMERA
// response means the packet CAMERA sends to MCU

/****************** Debug Configuration ******************/
#define ENABLE_DEBUG 0
#define NUMBER_CHARS_ONE_ROW 0x10

/****************** Camera Configuration ******************/
#define GROVE_UART_TX_PIN P4_28
#define GROVE_UART_RX_PIN P4_29
#define PIC_FORMAT PIC_FORMAT_VGA
#define PIC_CAMERA camera
#define CAMERA_ADDR 0

extern Serial debugcom;
/*************** Hardware Configuration ******************/
Serial camera(GROVE_UART_TX_PIN,GROVE_UART_RX_PIN);

const char cameraAddr = CAMERA_ADDR << 5;

void RxIntHandler(void)
{
		receive.completed = 0;
		while(camera.readable())
		{
				receive.buffer[receive.position++] = camera.getc();
		}
		return;
}

/*  picture package handler function */
int donothing(package_info *ppackage)
{
		return 0;
}

/***************************
@func 
@param
@auth
***************************/
int is_ack(char response[],unsigned short cmd,unsigned short length)
{
		if(length == 6)
				if(response[0] == 0xaa && response[1] == 0x0e && response[2] == cmd && response[4] == 0 && response[5] == 0)
						return true;
  	return false;
}
/***************************
@func 
@param
@auth
***************************/
void clearRxBuf()
{
		char temp;
    while (camera.readable())
    {
        temp = camera.getc();
				#if ENABLE_DEBUG
						debugcom.printf("%s %s %d camera.getc 0x%02x \n\r",__FILE__,__FUNCTION__,__LINE__,temp);
				#endif
    }
}

/***************************
@func send commmand to camera
@param buffer to send
@auth
***************************/
void sendCmd(char cmd[], int cmd_len)
{
    for (char i = 0; i < cmd_len; i++) 
		{
				#if ENABLE_DEBUG
						debugcom.printf("%s %s %d camera.putc 0x%02x \n\r",__FILE__,__FUNCTION__,__LINE__,cmd[i]);
				#endif
				camera.putc(cmd[i]);
		}
}
/*********************************************
*@func verifyPackage
*@param response buffer and number of received bytes
*@return -1 when failure
		  0 when success
*********************************************/
int verifyPackage(char response[],unsigned short size)
{
	unsigned short sum = 0;
	unsigned short i = 0;
	
	if(size > 512) //each package consists of maximum 512 bytes
		return -1;
	
	do{
		sum += response[i++];
	}while(i < size - 2);

	debugcom.printf("%s %s %d sum = 0x%04x \r\n",__FILE__,__FUNCTION__,__LINE__,sum);
	
	if(sum % 0x100 == response[size - 2])
		//response[510] ==>> lower byte of the sum
		return 0;
	else
		return -1;
}

/***************************
@func get response of command 
@param each retry means 5 ms more
@auth
@ret return the number of received bytes
***************************/
int getResponse(char **response,unsigned short length,unsigned short retry)
{		
		#if ENABLE_DEBUG
		int i = 0;
		#endif
		while(receive.position < length && retry > 0)
		{
				debugcom.printf("%s %s %d receive.position = %u .\r\n",
						__FILE__,__FUNCTION__,__LINE__,receive.position);
				wait_ms(5);
				retry--;
		}
		receive.completed = 1;
		receive.number = receive.position;
		receive.position = 0;
		*response = receive.buffer;
		#if ENABLE_DEBUG
		debugcom.printf("%s %s %d ",__FILE__,__FUNCTION__,__LINE__);
		for(i = 0;i<receive.number;i++)
				debugcom.printf("0x%02x ",receive.buffer[i]);
		debugcom.printf("\r\n");
		#endif
		return (int)receive.number ;
}

/***************************
@func 
@param
@auth
***************************/
void preCapture()
{
    char cmd[] = { 0xaa, 0x01 | cameraAddr, 0x00, 0x07, 0x00, PIC_FORMAT_VGA };
    unsigned char resp[12] = {0};
		#if ENABLE_DEBUG
			unsigned short i = 0;
		#endif

    //Serial.setTimeout(100);
    while (1)
    {
        clearRxBuf();
        sendCmd(cmd, 6);
				
				if(strlen((char *)resp) != 6)
						continue;
				#if ENABLE_DEBUG
						for( i = 0 ; i < sizeof(resp) ; i++)
								debugcom.printf("%s %s %d 0x%02x ",__FILE__,__FUNCTION__,__LINE__,resp[i]);
						debugcom.printf("\r\n");
				#endif
        if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x01 && resp[4] == 0 && resp[5] == 0) break;
    }
}
/***************************
@func make connection and will moodify cmd buffer
@param commanad buffer and command size
@auth
@return return 0 when success
				return -1 when no ACK
				return -2 when no SYNC command from camera
***************************/
int makeConnection(char cmd[],unsigned short length	)
{
		unsigned short retry = 60 ;
		#if RX_USE_INTERRUPT
		char *response = NULL;
		#else
		char response[10] = {0};
		#endif
		
		clearRxBuf();
		do
		{
			sendCmd(cmd,length);
			DEBUG_INFO(cmd,length);
			retry --;
		}while(getResponse(&response,12,5) != 12 && retry > 0);
		DEBUG_INFO(response,12);
		if(!is_ack(response,0x0d,6))
				return -1;
		if(response[6] == 0xaa && response[7] == 0x0d)
		{
				cmd[0] = 0xaa;
				cmd[1] = 0x0e;
				cmd[2] = 0x0d;
				cmd[3] = 0;
				cmd[4] = 0;
				cmd[5] = 0;
			sendCmd(cmd,6);
		}
		else
				return -2;
		return 0;
}
/***************************
@func send specific instructions to camera
@param commanad buffer and command size also the name of this command
@auth
@return return 0 when success
				return -1 when no ACK
***************************/
int instructCamera(char cmd[],unsigned short length,char *cmd_name)
{
		#if RX_USE_INTERRUPT
		char *response = NULL;
		#else
		char response[10] = {0};
		#endif
		
		//clearRxBuf();
		sendCmd(cmd,6);
		DEBUG_INFO(cmd,6);
		debugcom.printf("%s %s %d Send %s .\r\n",__FILE__,__FUNCTION__,__LINE__,cmd_name);
		//initialize command command code
		if(getResponse(&response,6,5) == 6 && is_ack(response,cmd[1],6))
		{
				DEBUG_INFO(response,6);
				return 0;
		}
		else
				return -1;
}
/***************************
@func 
@param
@auth
***************************/
void initialize()
{
		int ret = 0;
		char cmd[10] = {0xaa,0x0d,0,0,0,0};
		
		OE = 1;
		
		receive.completed = 0;  //the index of the next package
		receive.number = 0;
		receive.position = 0;
		memset(receive.buffer,0,sizeof(receive.buffer));
		
		camera.baud(115200);
		camera.attach(&RxIntHandler,Serial::RxIrq);
		
		ret = makeConnection(cmd,6);
		if(ret == 0){
				debugcom.printf("Make Connection done .\r\n");
		}
		else if(ret == -1){
				debugcom.printf("No ACK from camera .\r\n");
		}
		else{
				debugcom.printf("No SYNC command from camera .\r\n");
		}

		return;
		
}
/***************************
@func 
@param
@auth
***************************/
void resetCamera()
{
		int i = 0;
	
		serial_camera_cmd command[]={
				{0xaa,0x08,0x00,0x00,0x00,0x00,"Reset command",0},
		};
		
		for( i=0 ; i<sizeof(command)/sizeof(serial_camera_cmd) ; i++)
		{
				command[i].ret = instructCamera((char *)&(command[i].header),6,command[i].name);
				if(command[i].ret == 0)
						debugcom.printf("%s done .\r\n",command[i].name);
				else
						debugcom.printf("No ACK from camera .\r\n");
		}		
}
/***************************
@func get JPEG snapshot
@param 
@return return 0 when success
				return -1 when No picture size received
				return -2 when Data Missed
				return -3 when invalid parameter
				return -4 when open file failed
@auth
***************************/
int cameraSnapshot(picture_info *pInfo)
{
		int i = 0,ret = 0,bytes_written = 0;
		unsigned long sum = 0;
		char *response = NULL;
		char cmd[10] = {0};
		unsigned char jpeg_resolution = 0;
		unsigned char uncompressed = 0;
		FILE *fp = fopen(pInfo->name,"w+");
		if(!fp){
			debugcom.printf("%s %s %d Open file %s failed .",
				__FILE__,__FUNCTION__,__LINE__,pInfo->name);
			return -4;
		}

		if(pInfo->width == 320 && pInfo->height == 240)
			jpeg_resolution = 0x05;
		else if(pInfo->width == 640 && pInfo->height == 480)
			jpeg_resolution  = 0x07;
		else{
			debugcom.printf("Invalid JPEG resolution .\r\n");
			return -3;
		}	

		if(pInfo->is_compressed)
			uncompressed = 0;
		else
			uncompressed = 1;
			
		serial_camera_cmd command[]={
				{0xaa,0x01,0x00,0x07,0x07,jpeg_resolution,"Init command",0},
				{0xaa,0x07,0x00,0x01,0x00,0x00,"Set Baud rate 115200",0},
				{0xaa,0x06,0x08,0x00,0x02,0x00,"Set Package Size 512B",0},
				{0xaa,0x05,uncompressed,0x00,0x00,0x00,"Snapshot compressed picture",0},
				{0xaa,0x04,pInfo->type,0x00,0x00,0x00,"Get Picture Data",0},
				//{0xaa,0x0a,0x01,0x00,0x00,0x00,"Get Picture Data",0},
		};
		
		for( i=0 ; i<sizeof(command)/sizeof(serial_camera_cmd) ; i++)
		{
				command[i].ret = instructCamera((char *)&(command[i].header),6,command[i].name);
				if(command[i].ret == 0)
						debugcom.printf("%s done .\r\n",command[i].name);
				else
						debugcom.printf("No ACK from camera .\r\n");
		}		
		
		//get the size of current picture
		if(getResponse(&response,6,5) == 6 && response[0] == 0xaa && response[1] == COMMAND_DATA)
		{
				unsigned char retry = 5;
				DEBUG_INFO(response,6);
				pInfo->size = response[3] | (response[4] << 8) | (response[5] << 16);
				debugcom.printf("Data size received from camera 0x%02x 0x%02x 0x%02x picture size = 0x%08x .\r\n",
						response[3],response[4],response[5],pInfo->size);
				while(true)
				{
						cmd[0] = 0xaa;
						cmd[1] = 0x0e;
						cmd[2] = 0x00;
						cmd[3] = 0x00;
						cmd[4] = pInfo->package->index & 0xff;
						cmd[5] = (pInfo->package->index & 0xff00) >> 8;
						sendCmd(cmd,6);
						DEBUG_INFO(cmd,6);
					
						ret = getResponse(&response,pInfo->package->size,5);
						
						debugcom.printf("%s %s %d received %d bytes package index %d .\r\n",
								__FILE__,__FUNCTION__,__LINE__,ret,pInfo->package->index); 
						for(i=0;i<ret/16+(ret%16==0 ? 0 : 1);i++)
							DEBUG_INFO(&response[16*i], (ret - 16*i) >= 16 ? 16 : ret - 16*i);

						if(verifyPackage(response,/*pInfo->package->size*/ret) == -1){
							retry--;
							if(retry == 0){
								debugcom.printf("%s %s %d Package Verfy failed .",
									__FILE__,__FUNCTION__,__LINE__);
								fclose(fp);
								if ( 0 == remove(pInfo->name))
									debugcom.printf("%s %s %d file removed \r\n",
										__FILE__,__FUNCTION__,__LINE__,pInfo->name);
								return -2;
							}
							else
								continue;
						}
						debugcom.printf("%s %s %d Package Verfy succeeded .\r\n",
							__FILE__,__FUNCTION__,__LINE__);

						bytes_written = fwrite(&response[4],sizeof(unsigned char),ret - 6,fp);
						debugcom.printf("%s %s %d %d bytes written into file %s .\r\n",
							__FILE__,__FUNCTION__,__LINE__,bytes_written,pInfo->name);

						retry = 5;

						pInfo->package->index ++;
						sum += ret;
					
						//sleep and waiting to be waken up (TODO)
						//call package handler
						pInfo->phandler(pInfo->package);		
					
						if(pInfo->package->index  >= ( pInfo->size/(pInfo->package->size - 6) + (pInfo->size%(pInfo->package->size - 6)> 0 ? 1 : 0)))
								break;
				}

				fclose(fp);
				
				cmd[0] = 0xaa;
				cmd[1] = 0x0e;
				cmd[2] = 0x00;
				cmd[3] = 0x00;
				cmd[4] = 0xf0;
				cmd[5] = 0xf0;
				sendCmd(cmd,6);
				DEBUG_INFO(cmd,6);
				
				if(sum == pInfo->size +  pInfo->package->index * 6)
						return 0;
				else
						return -2;

		}
		else
		{
				DEBUG_INFO(response,6);
				fclose(fp);
				remove(pInfo->name);
				return -1;
		}

}

/***************************
@func get JPEG snapshot
@param 
@return return 0 when success
				return -1 when No picture size received
				return -2 when Data Missed
				return -3 when invalid parameter
				return -4 when open file failed
@auth
***************************/
int cameraGo(picture_info *pInfo)
{
		int i = 0,ret = 0,bytes_written = 0;
		unsigned long sum = 0;
		char *response = NULL;
		char cmd[10] = {0};
		FILE *fp = fopen(pInfo->name,"w");
		if(!fp){
			debugcom.printf("%s %s %d Open file %s failed .",
				__FILE__,__FUNCTION__,__LINE__,pInfo->name);
			return -4;
		}
			
		serial_camera_cmd command[]={
				{0xaa,0x07,0x0f,0x01,0x00,0x00,"Set Baud rate 115200",0},
				{0xaa,0x10,0x00,0x06,0x00,0x00,"Set best quality",0},
				{0xaa,0x01,0x00,0x00,0x00,0x05,"Set Image size 320X240",0},
				{0xaa,0x04,0x05,0x00,0x00,0x00,"Camera Take Photo",0},
		};
		
		for( i=0 ; i<sizeof(command)/sizeof(serial_camera_cmd) ; i++)
		{
				command[i].ret = instructCamera((char *)&(command[i].header),6,command[i].name);
				if(command[i].ret == 0)
						debugcom.printf("%s done .\r\n",command[i].name);
				else
						debugcom.printf("No ACK from camera .\r\n");
		}		
		
		//get the size of current picture
		if(getResponse(&response,6,1000) == 6 && response[0] == 0xaa && response[1] == COMMAND_DATA && response[2] == 0x05)
		{
				unsigned char retry = 5;
				DEBUG_INFO(response,6);
				pInfo->size = response[3] | (response[4] << 8) | (response[5] << 16);
				debugcom.printf("Data size received from camera 0x%02x 0x%02x 0x%02x picture size = 0x%08x .\r\n",
						response[3],response[4],response[5],pInfo->size);
				while(true)
				{
						cmd[0] = 0xaa;
						cmd[1] = 0x0e;
						cmd[2] = 0x00;
						cmd[3] = 0x00;
						cmd[4] = pInfo->package->index & 0xff;
						cmd[5] = (pInfo->package->index & 0xff00) >> 8;
						sendCmd(cmd,6);
						DEBUG_INFO(cmd,6);
					
						ret = getResponse(&response,pInfo->package->size,5);
						
						debugcom.printf("%s %s %d received %d bytes package index %d .\r\n",
								__FILE__,__FUNCTION__,__LINE__,ret,pInfo->package->index); 

						if(verifyPackage(response,/*pInfo->package->size*/ret) == -1){
							retry--;
							if(retry == 0){
								debugcom.printf("%s %s %d Package Verfy failed .",
									__FILE__,__FUNCTION__,__LINE__);
								fclose(fp);
								if ( 0 == remove(pInfo->name))
									debugcom.printf("%s %s %d file removed \r\n",
										__FILE__,__FUNCTION__,__LINE__,pInfo->name);
								return -2;
							}
							else
								continue;
						}
						debugcom.printf("%s %s %d Package Verfy succeeded .\r\n",
							__FILE__,__FUNCTION__,__LINE__);

						bytes_written = fwrite(&response[4],sizeof(unsigned char),ret - 6,fp);
						debugcom.printf("%s %s %d %d bytes written into file %s .\r\n",
							__FILE__,__FUNCTION__,__LINE__,bytes_written,pInfo->name);

						retry = 5;

						pInfo->package->index ++;
						sum += ret;
					
						//sleep and waiting to be waken up (TODO)
						//call package handler
						pInfo->phandler(pInfo->package);		
					
						if(pInfo->package->index  >= ( pInfo->size/(pInfo->package->size - 6) + (pInfo->size%(pInfo->package->size - 6)> 0 ? 1 : 0)))
								break;
				}

				fclose(fp);
				
				cmd[0] = 0xaa;
				cmd[1] = 0x0e;
				cmd[2] = 0x00;
				cmd[3] = 0x00;
				cmd[4] = 0xf0;
				cmd[5] = 0xf0;
				sendCmd(cmd,6);
				DEBUG_INFO(cmd,6);
				
				if(sum == pInfo->size +  pInfo->package->index * 6)
						return 0;
				else
						return -2;

		}
		else
		{
				DEBUG_INFO(response,6);
				fclose(fp);
				remove(pInfo->name);
				return -1;
		}

}

/***************************
@func get picture data from camera
@param data buffer and data sbuffer size 
@auth
@return return number of package received
				return -1 when no ACK
***************************/				
int getPictureData(picture_info *pInfo)
{
		#if RX_USE_INTERRUPT
		char *response = NULL;
		#else
		char response[10] = {0};
		#endif
		int i = 0;
		int ret = 0;
		
		//get the size of current picture
		if(getResponse(&response,6,2000) == 6 && response[0] == 0xaa && response[1] == COMMAND_DATA)
		{
				DEBUG_INFO(response,6);
				debugcom.printf("Data size received from camera 0x%02x 0x%02x 0x%02x .\r\n",
						response[3],response[4],response[5]);
				pInfo->size = response[3] | (response[4] << 8) | (response[5] << 16);
				while(true)
				{
						response[0] = 0xaa;
						response[1] = 0x0e;
						response[2] = 0x00;
						response[3] = 0x00;
						response[4] = pInfo->package->index & 0xff;
						response[5] = (pInfo->package->index & 0xff00) >> 8;
						sendCmd(response,6);
						DEBUG_INFO(response,6);
					
						//fill the buffer
						ret = getResponse(&response,pInfo->package->size,1000);	
						debugcom.printf("%s %s %d received %d data .\r\n",
								__FILE__,__FUNCTION__,__LINE__,ret); 
						//camera.gets(pInfo->package->buffer,pInfo->package->size-1);
					
						//increase the index of the buffer by 1
						pInfo->package->index ++;		
					
						//sleep and waiting to be waken up (TODO)
						//call package handler
						pInfo->phandler(pInfo->package);		
					
						#if 1
						for(i=0;i<pInfo->package->size/NUMBER_CHARS_ONE_ROW;i++)
								DEBUG_INFO((&(response[i*NUMBER_CHARS_ONE_ROW])),NUMBER_CHARS_ONE_ROW)
						#endif 
					
					  if(pInfo->package->index + 1 >= pInfo->size/pInfo->package->size)
								break;
				}

		}
		else
		{
				DEBUG_INFO(response,6);
				debugcom.printf("No data size recieved .\r\n");
		}
		return 0;
}


