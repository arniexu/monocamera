#ifndef JOYSTICK_H
#define JOYSTICK_H

typedef struct __joystick{
	unsigned short horz;
	unsigned short vert;
	unsigned short button;
	unsigned short direction;
}_Joystick;

#define NONE  0
#define RIGHT 1
#define LEFT  2
#define UP    3
#define DOWN  4

void initialize_joystick();
bool isDirectionValid(const char * direction);
void joystick_Int_Handler();

#endif
