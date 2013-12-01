#include "vga.h"
/** 
 * Start VGA output.
 */
void vga_start(void){
	//Start vsync timer that starts vga.
	tc_start(TC_VGA, TCC_VSYNC);
}

/** 
 * Stop VGA output.
 */
void vga_stop(void){
	//Stop vsync timer
	tc_stop(TC_VGA, TCC_VSYNC);
	//Stop vsync pulse timer
	tc_stop(TC_VGA, TCC_VSYNC_PULSE);
	//Stop hsync timer
	tc_stop(TC_VGA, TCC_HSYNC);
}


/** 
 * Initialization of vga output.
 */
void vga_init(void){
	//Enable timer clocks.
	sysclk_enable_peripheral_clock(ID_TC0);
	sysclk_enable_peripheral_clock(ID_TC1);
	sysclk_enable_peripheral_clock(ID_TC2);
	
	//Initialize vsync timer.
	tc_set_writeprotect(TC_VGA, 0);
	tc_init(TC_VGA, TCC_VSYNC, TC_CMR_TCCLKS_TIMER_CLOCK4 | TC_CMR_CPCTRG | TC_CMR_ACPC_CLEAR);
	//Set Compare register value to fire interrupt every 1/60.04 seconds.
	tc_write_rc(TC_VGA, TCC_VSYNC, VSYNC_PULSE);
	//Enable compare interrupt.
	tc_enable_interrupt(TC_VGA, TCC_VSYNC, TC_IER_CPCS);
	
	//Initialize vsync pulse length timer.
	tc_set_writeprotect(TC_VGA, 0);
	tc_init(TC_VGA, TCC_VSYNC_PULSE, TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_CPCTRG | TC_CMR_ACPC_CLEAR);
	//Set Compare register value to fire interrupt every 2/60/524 seconds(The length of 2 lines = length of vsync pulse).
	tc_write_rc(TC_VGA, TCC_VSYNC_PULSE, VSYNC_PULSE_LEN);
	//Enable compare interrupt.
	tc_enable_interrupt(TC_VGA, TCC_VSYNC_PULSE, TC_IER_CPCS);
	
	//Setup vsync pin
	ioport_enable_pin(PIN_VSYNC);
	ioport_set_pin_dir(PIN_VSYNC, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_VSYNC, 0);
	
	//Initialize hsync timer.
	tc_set_writeprotect(TC_VGA, 0);
	tc_init(TC_VGA, TCC_HSYNC, TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_CPCTRG | TC_CMR_ACPC_CLEAR);
	//Set Compare register value to fire interrupt every 1/60/524 seconds.
	tc_write_rc(TC_VGA, TCC_HSYNC, HSYNC_PULSE);
	//Enable compare interrupt.
	tc_enable_interrupt(TC_VGA, TCC_HSYNC, TC_IER_CPCS);
	
	//Setup hsync pin
	ioport_enable_pin(PIN_HSYNC);
	ioport_set_pin_dir(PIN_HSYNC, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_HSYNC, 0);
	hsync_port = arch_ioport_pin_to_base(PIN_HSYNC);
	hsync_mask = arch_ioport_pin_to_mask(PIN_HSYNC);

	//Get RGB port, all rgb pins should be on the same port.
	rgb_port = arch_ioport_pin_to_base(PIN_RED_1);

	//Setup red pins
	ioport_enable_pin(PIN_RED_1);
	ioport_set_pin_dir(PIN_RED_1, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_RED_1, 0);
	red_mask_1 = arch_ioport_pin_to_mask(PIN_RED_1);
	ioport_enable_pin(PIN_RED_2);
	ioport_set_pin_dir(PIN_RED_2, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_RED_2, 0);
	red_mask_2 = arch_ioport_pin_to_mask(PIN_RED_2);
	ioport_enable_pin(PIN_RED_3);
	ioport_set_pin_dir(PIN_RED_3, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_RED_3, 0);
	red_mask_3 = arch_ioport_pin_to_mask(PIN_RED_3);
	
	//Setup green pins
	ioport_enable_pin(PIN_GREEN_1);
	ioport_set_pin_dir(PIN_GREEN_1, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_GREEN_1, 0);
	green_mask_1 = arch_ioport_pin_to_mask(PIN_GREEN_1);
	ioport_enable_pin(PIN_GREEN_2);
	ioport_set_pin_dir(PIN_GREEN_2, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_GREEN_2, 0);
	green_mask_2 = arch_ioport_pin_to_mask(PIN_GREEN_2);
	ioport_enable_pin(PIN_GREEN_3);
	ioport_set_pin_dir(PIN_GREEN_3, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_GREEN_3, 0);
	green_mask_3 = arch_ioport_pin_to_mask(PIN_GREEN_3);
	
	//Setup blue pins
	ioport_enable_pin(PIN_BLUE_1);
	ioport_set_pin_dir(PIN_BLUE_1, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_BLUE_1, 0);
	blue_mask_1 = arch_ioport_pin_to_mask(PIN_BLUE_1);
	ioport_enable_pin(PIN_BLUE_2);
	ioport_set_pin_dir(PIN_BLUE_2, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_BLUE_2, 0);
	blue_mask_2 = arch_ioport_pin_to_mask(PIN_BLUE_2);

	rgb_mask = red_mask_3 | red_mask_2 | red_mask_1 | green_mask_3
	| green_mask_2 | green_mask_1 | blue_mask_2 | blue_mask_1;

	//Enable IRQ for timers.
	NVIC_ClearPendingIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn, 0);
	NVIC_SetPriority(TC1_IRQn, 0);
	NVIC_SetPriority(TC2_IRQn, 0);
	NVIC_EnableIRQ(TC0_IRQn);
	NVIC_EnableIRQ(TC1_IRQn);
	NVIC_EnableIRQ(TC2_IRQn);
	
	//Initialize pixel buffer.
	vga_clear_screen(COLOR_BLACK);
}


/** 
 * Interrupt handler for a vsync signal.
 * Starts a vsync pulse.
 */
void vga_vsync_Handler(void){
	
	if(tc_get_status(TC_VGA, TCC_VSYNC) & TC_IER_CPCS){
		//Start vsync pulse
		tc_start(TC_VGA, TCC_VSYNC_PULSE);
		ioport_set_pin_level(PIN_VSYNC, 0);
		
		//Debug 1 second led counter.
		sec_counter++;
		if(sec_counter == 60){
			active = !active;
			ioport_set_pin_level(LED_0_PIN, active);
			sec_counter = 0;
		}
	}
	return;
}

/** 
 * Interrupt handler for vsync signal end.
 * Disables vsync pulse and starts hsyncs.
 */
void vga_vsync_pulse_Handler(void){
	if(tc_get_status(TC_VGA, TCC_VSYNC_PULSE) & TC_IER_CPCS){
		//Stop vsync pulse
		tc_stop(TC_VGA, TCC_VSYNC_PULSE);
		ioport_set_pin_level(PIN_VSYNC, 1);
		hsync_counter = 0;
		//Start hsync
		tc_start(TC_VGA, TCC_HSYNC);
		vga_hsync();
	}
	return;
}

/** 
 * Interrupt handler for an hsync.
 */
void vga_hsync_Handler(void){
	if(tc_get_status(TC_VGA, TCC_HSYNC) & TC_IER_CPCS){
		vga_hsync();
	}
	return;
}

/** 
 * hsync body, draws one horizontal line.
 */
void vga_hsync(){
	//Enable hsync pulse.
	hsync_port->PIO_CODR = hsync_mask;
	SLEEP200
	SLEEP200
	SLEEP20
	SLEEP20
	SLEEP10
	SLEEP5
	hsync_port->PIO_SODR = hsync_mask;
	
	hsync_counter++;
	//If we have drawn 524+ lines, disable hsync timer.
	if(hsync_counter >= 524){
		tc_stop(TC_VGA, TCC_HSYNC);
	}
	//Check back porch first so both expressions are always evaluated.
	if(hsync_counter >= VSYNC_BACK_PORCH || hsync_counter < VSYNC_FRONT_PORCH){
		return;
	}

//Blits one pixel.
#define BLIT_PIXEL(pixel) rgb_port->PIO_ODSR = (*pixel)<<17; pixel++; SLEEP1 SLEEP1 SLEEP1
//Macros to blit multiple pixels, Instead of loop we write out all pixels
//to reduce overhead of loops.
#define BLIT_5_PIXELS(pixel) BLIT_PIXEL(pixel) BLIT_PIXEL(pixel) BLIT_PIXEL(pixel) BLIT_PIXEL(pixel) BLIT_PIXEL(pixel)
#define BLIT_25_PIXELS(pixel) BLIT_5_PIXELS(pixel) BLIT_5_PIXELS(pixel) BLIT_5_PIXELS(pixel) BLIT_5_PIXELS(pixel) BLIT_5_PIXELS(pixel)
#define BLIT_100_PIXELS(pixel) BLIT_25_PIXELS(pixel) BLIT_25_PIXELS(pixel) BLIT_25_PIXELS(pixel) BLIT_25_PIXELS(pixel)
	
	int line = (hsync_counter - VSYNC_FRONT_PORCH)/2;
	unsigned char* pixel = pixel_buffer[line];
	
	//Horizontal front porch.
	SLEEP200
	SLEEP20
	
	//Blit pixel buffer.
	BLIT_100_PIXELS(pixel)
	BLIT_100_PIXELS(pixel)
	BLIT_100_PIXELS(pixel)
	BLIT_5_PIXELS(pixel)
	BLIT_5_PIXELS(pixel)
	BLIT_5_PIXELS(pixel)
	BLIT_5_PIXELS(pixel)
	
	//Disable any colors in use.
	rgb_port->PIO_CODR = rgb_mask;
	
	//Horizontal back porch, exists for debug reasons.
	//SLEEP100
	//SLEEP20
	
	return;
}

/** 
 * Clear screen to given color.
 */
void vga_clear_screen(unsigned char color){
	for(int r = 0; r < SCREEN_HEIGHT; r++){
		for(int c = 0; c < SCREEN_WIDTH; c++){
			pixel_buffer[r][c] = color;
		}
	}
}

void vga_set_pixel(unsigned int x, unsigned int y, unsigned char color){
	pixel_buffer[y][x] = color;
}

unsigned char vga_get_pixel(unsigned int x, unsigned int y){
	return pixel_buffer[y][x];
}