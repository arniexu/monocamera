#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "button.h"

menuinfo menu[2];

void initialize_menu()
{

	unsigned char i = 0;
	menu[0].ID = 0;
	for(i=0;i<6;i++)
	{
		menu[0].items[i].ID = i;
		menu[0].items[i].x = 240;
		menu[0].items[i].y = 24*i;
		menu[0].items[i].selected = (i==0?true:false);
		menu[0].items[i].content.bmpinfo.xSize = 24;
		menu[0].items[i].content.bmpinfo.ySize = 24;
		menu[0].items[i].content.bmpinfo.Byte_in_Line = 3;
	}
	menu[0].items[0].content.bmpinfo.data = (char *)buttonA;
	//menu[0].items[1].content.bmpinfo.data = buttonB;

	menu[1].ID = 1;
	for(i=0;i<6;i++)
	{
		menu[0].items[i].ID = i;
		//suppose the string is displayed in y direction
		//suppose the character is 24x24
		//the string starts at y = 176 ends at y = 0
		menu[1].items[i].x = CHAR_SIZE*i;
		menu[1].items[i].y = 176;
		menu[1].items[i].selected = (i==0?true:false);
		if(i==5)
		{
			menu[1].items[i].content.bmpinfo.xSize = 48;
			menu[1].items[i].content.bmpinfo.ySize = 176;
			menu[1].items[i].content.bmpinfo.Byte_in_Line = 3;
			//menu[1].items[i].content.bmpinfo.data = buttonPlus;
		}
		else
			strcpy((char *)menu[1].items[i].content.filename,"empty file");
	}
}

