#include "debug.h"
#include "mbed.h"

extern Serial camera;
extern Serial debugcom;

void debuginfo(char *buffer,unsigned short length)
{
		unsigned short i = 0;
		for(i=0 ; i<length ; i++)
				debugcom.printf("0x%02x ",buffer[i]);
		debugcom.printf("\r\n");
}

