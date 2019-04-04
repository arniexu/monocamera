#ifndef MENU_H
#define MENU_H

#include "EaEpaper.h"

#define ICON_SIZE 24
#define CHAR_SIZE 24

typedef struct __iteminfo{
	unsigned char ID;
	unsigned char selected;
	unsigned char x; //x location from 0-263
 	unsigned char y; //y location from 0-175
	union {
		struct Bitmap bmpinfo;
		unsigned char filename[16];
	}content;
		
}iteminfo;

typedef struct __menuinfo{
	unsigned char ID;
	iteminfo items[6];
}menuinfo;

#endif
