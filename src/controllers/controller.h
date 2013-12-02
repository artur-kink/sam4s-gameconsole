#ifndef _CONTROLLER_
#define _CONTROLLER_

#include "asf.h"

unsigned char controller_num_buttons;
unsigned char num_controllers;
unsigned short* controller_button_states;
unsigned short* controller_buttons_pressed;
unsigned short* controller_buttons_released;

void controller_init(void);
void controller_update(void);

inline unsigned short controller_is_down(unsigned char controller, unsigned char button){
	return controller_button_states[controller] & (1 << button);
}

inline unsigned short controller_is_pressed(unsigned char controller, unsigned char button){
	return controller_buttons_pressed[controller] & (1 << button);
}

inline unsigned short controller_is_released(unsigned char controller, unsigned char button){
	return controller_buttons_released[controller] & (1 << button);
}

#endif