#include "controller.h"
#include "ps_controller.h"

unsigned int** ps_buttons;

/**
 * Initialize playstation controllers.
 */
void controller_init(void){
	
	num_controllers = 2;
	
	controller_button_states = malloc(sizeof(unsigned short)*num_controllers);
	controller_buttons_pressed = malloc(sizeof(unsigned short)*num_controllers);
	controller_buttons_released = malloc(sizeof(unsigned short)*num_controllers);
	
	controller_num_buttons = 10;
	
	ps_buttons = malloc(sizeof(unsigned int*)*num_controllers);
		ps_buttons[0] = malloc(sizeof(unsigned int)*controller_num_buttons);
		ps_buttons[0][0] = EXT3_PIN_7;
		ps_buttons[0][1] = EXT3_PIN_8;
		ps_buttons[0][2] = EXT3_PIN_9;
		ps_buttons[0][3] = EXT3_PIN_10;
		ps_buttons[0][4] = EXT3_PIN_11;
		ps_buttons[0][5] = EXT3_PIN_12;
		ps_buttons[0][6] = EXT3_PIN_13;
		ps_buttons[0][7] = EXT3_PIN_14;
		ps_buttons[0][8] = EXT3_PIN_15;
		ps_buttons[0][9] = EXT3_PIN_16;
		
		ps_buttons[1] = malloc(sizeof(unsigned int)*controller_num_buttons);
		ps_buttons[1][0] = EXT3_PIN_17;
		ps_buttons[1][1] = EXT3_PIN_18;
		ps_buttons[1][2] = EXT2_PIN_9;
		ps_buttons[1][3] = EXT2_PIN_10;
		ps_buttons[1][4] = EXT3_PIN_7;
		ps_buttons[1][5] = EXT3_PIN_7;
		ps_buttons[1][6] = EXT3_PIN_7;
		ps_buttons[1][7] = EXT3_PIN_7;
		ps_buttons[1][8] = EXT3_PIN_7;
		ps_buttons[1][9] = EXT3_PIN_7;

	int i = 0;
	int c = 0;
	for(c = 0; c < num_controllers; c++){
		for(i = 0; i < controller_num_buttons; i++){
			ioport_enable_pin(ps_buttons[c][i]);
			ioport_set_pin_level(ps_buttons[c][i], 0);
			ioport_set_pin_dir(ps_buttons[c][i], IOPORT_DIR_INPUT);
		}
		controller_button_states[c] = 0;
	}
	
}

/**
 * Update controller states.
 */
void controller_update(void){
	int i = 0;
	int c = 0;
	for(c = 0; c < num_controllers; c++){
		controller_button_states[c] = 0;
		for(i = 0; i < controller_num_buttons; i++){
			controller_button_states[c] = (controller_button_states[c] | (ioport_get_pin_level(ps_buttons[c][i]) << i));
		}
	}
}