#include "mbed.h"
#include "joystick.h"
#include "debug.h"

_Joystick input;
Ticker joystick; 
extern Serial debugcom;

//Joystick hardware mapping
AnalogIn xAxis(A5);
AnalogIn yAxis(A4);

void initialize_joystick()
{
		input.horz = 700;
		input.vert = 700;
		input.direction = NONE;
		input.button = NONE;
		joystick.attach(joystick_Int_Handler,0.2);
}

bool isDirectionValid(const char * direction)
{
		if(strncmp(direction,"up",strlen("up")) == 0 )
			if(input.vert > 900)
			{
				wait_ms(20);
				if(input.vert > 900)
					return true;
			}
		if(strncmp(direction,"down",strlen("down")) == 0 )
			if(input.vert < 550)
			{
				wait_ms(20);
				if(input.vert < 550)
					return true;
			}
		if(strncmp(direction,"right",strlen("right")) == 0 )
			if(input.horz > 900)
			{
				wait_ms(20);
				if(input.horz > 900)
					return true;
			}
		if(strncmp(direction,"left",strlen("left")) == 0 )
			if(input.horz < 550)
			{
				wait_ms(20);
				if(input.horz < 550)
					return true;
			}
		return false;
}
//Joystick interrupt handler
void joystick_Int_Handler()
{
		static int i = 0;
		//CONVERT float value 0-1 to unsigned short 0-1000
    input.horz = (unsigned short)(xAxis.read() * 1000); 
    input.vert = (unsigned short)(yAxis.read() * 1000);
    if (isDirectionValid("right"))
	{
		debugcom.printf("Joystick input direction RIGHT detected %d \n\r",i++);
		input.direction = RIGHT;
	}
    else if(isDirectionValid("left"))
	{
		debugcom.printf("Joystick input direction LEFT detected %d \n\r",i++);
		input.direction = LEFT;
	}
	else if (isDirectionValid("up"))
	{
		debugcom.printf("Joystick input direction UP detected %d \n\r",i++);
		input.direction = UP;
	}
	else if (isDirectionValid("down"))
	{
		debugcom.printf("Joystick input direction DOWN detected %d \n\r",i++);
		input.direction = DOWN;
	}
	else
			input.direction = NONE;
}
