#ifndef _CONTROLLER_
#define _CONTROLLER_

#include "asf.h"

//Number of buttons on controller.
unsigned char controller_num_buttons;
//Number of controllers.
unsigned char num_controllers;
//Current button state for controllers.
unsigned short* controller_button_states;

unsigned short* controller_buttons_pressed;
unsigned short* controller_buttons_released;

void controller_init(void);
void controller_update(void);

/**
 * Check if button is being pressed on given controller.
 */
inline unsigned short controller_is_down(unsigned char controller, unsigned char button){
	return controller_button_states[controller] & (1 << button);
}

/**
 * Check if button was pressed this frame on given controller.
 * NOT IMPLEMENTED YET.
 */
inline unsigned short controller_is_pressed(unsigned char controller, unsigned char button){
	return controller_buttons_pressed[controller] & (1 << button);
}

/**
 * Check if button was released this frame on given controller.
 * NOT IMPLEMENTED YET.
 */
inline unsigned short controller_is_released(unsigned char controller, unsigned char button){
	return controller_buttons_released[controller] & (1 << button);
}

#endif