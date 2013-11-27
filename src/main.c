#include "asf.h"

//Macro to sleep 1 clock cycle.
#define SLEEP1 asm("nop");
//Sleep 5 clock cycles.
#define SLEEP5 SLEEP1 SLEEP1 SLEEP1 SLEEP1 SLEEP1
//Sleep 10 clock cycles.
#define SLEEP10 SLEEP5 SLEEP5
//Sleep 20 clock cycles.
#define SLEEP20 SLEEP10 SLEEP10
//Sleep 50 clock cycles.
#define SLEEP50 SLEEP20 SLEEP20 SLEEP10
//Sleep 100 clock cycles.
#define SLEEP100 SLEEP50 SLEEP50
//Sleep 200 clock cycles.
#define SLEEP200 SLEEP100 SLEEP100

//VGA Info
//640x480 60hz
//Field Rate: 60.04hz
//System Clock: 120mhz - 120,000,000hz
//Vsync Timer Speed: 120mhz/128 = 937,500hz
//Vsync pulse: 60.04hz = 937,000/60.04 = 15,614.59
#define VSYNC_PULSE 15614

//Vsync pulse length: 63.57mu
//Vsync Timer Speed: 120mhz/2 = 60mhz = 60hz/mu
//Vsync pulse length: 60x63.57 = 3,814
#define VSYNC_PULSE_LEN 3814

//Hsync pulse: 31.46khz
//Hsync Timer Speed: 120mhz/2 = 60mhz = 60,000khz
//Hsync pulse length: 60,000/31.46 = 1,907.18
#define HSYNC_PULSE 1907//1907

//Hsync pulse length: 3.81mu
//Hsync Timer Speed: 120mhz/2 = 60mhz = 60hz/mu
//Vsync pulse length: 60x 3.81 = 228.6
#define HSYNC_PULSE_LEN 228

//VGA Timer
#define TC_VGA TC0
//Vertical Sync Channel
#define TCC_VSYNC 0
//Vertical Sync Pulse Length Channel
#define TCC_VSYNC_PULSE 1
//Horizontal Sync Channel
#define TCC_HSYNC 2

//VSync Pin
#define PIN_VSYNC EXT3_PIN_3

//HSync Pin
#define PIN_HSYNC EXT3_PIN_5
//Hsync port
static Pio* hsync_port;
//Hsync pin mask in port
static unsigned int hsync_mask;

//RGB port, The port where the rgb pins are.
static Pio* rgb_port;
static unsigned int rgb_mask;

//Red Pins
#define PIN_RED_1 EXT1_PIN_5
static unsigned int red_mask_1;
#define PIN_RED_2 EXT1_PIN_7
static unsigned int red_mask_2;
#define PIN_RED_3 EXT2_PIN_14
static unsigned int red_mask_3;

//Green Pins
#define PIN_GREEN_1 EXT2_PIN_13
static unsigned int green_mask_1;
#define PIN_GREEN_2 EXT2_PIN_8
static unsigned int green_mask_2;
#define PIN_GREEN_3 EXT1_PIN_8
static unsigned int green_mask_3;

//Blue Pins
#define PIN_BLUE_1 EXT1_PIN_4
static unsigned int blue_mask_1;
#define PIN_BLUE_2 EXT1_PIN_3
static unsigned int blue_mask_2;

//Bitmask for Red
#define COLOR_RED 32+64+128
//Bitmask for Green
#define COLOR_GREEN 4+8+16
//Bitmask for Blue
#define COLOR_BLUE 1+2

//Bitmasks for additive colors.
#define COLOR_MAGENTA (COLOR_RED + COLOR_BLUE)
#define COLOR_TEAL (COLOR_GREEN + COLOR_BLUE)
#define COLOR_YELLOW (COLOR_RED + COLOR_GREEN)
#define COLOR_WHITE (COLOR_RED + COLOR_GREEN + COLOR_BLUE)
#define COLOR_BLACK 0

//Height of screen, the number of lines.
#define SCREEN_HEIGHT 240
//Width of screen, number of pixels per line.
#define SCREEN_WIDTH 320

//Screen pixel buffer.
unsigned char pixel_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];

//Vsync porch counts
#define VSYNC_FRONT_PORCH 12
#define VSYNC_BACK_PORCH 524-32

//Horizontal sync counter.
static int hsync_counter = 0;

//Debug LED state.
unsigned long active = LED_0_ACTIVE;
//Debug second counter.
static unsigned int sec_counter = 0;

void hsync(void);

/** 
 * Interrupt handler for a vsync signal.
 * Starts a vsync pulse.
 */
void TC_VSYNC_Handler(void){
	
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
void TC_VSYNC_PULSE_Handler(void){
	if(tc_get_status(TC_VGA, TCC_VSYNC_PULSE) & TC_IER_CPCS){
		//Stop vsync pulse
		tc_stop(TC_VGA, TCC_VSYNC_PULSE);
		ioport_set_pin_level(PIN_VSYNC, 1);
		hsync_counter = 0;
		//Start hsync
		tc_start(TC_VGA, TCC_HSYNC);
		hsync();
	}
	return;
}

/** 
 * Interrupt handler for an hsync.
 */
void TC_HSYNC_Handler(void){
	if(tc_get_status(TC_VGA, TCC_HSYNC) & TC_IER_CPCS){
		hsync();
	}
	return;
}

/** 
 * hsync handler, draws one horizontal line.
 */
void hsync(){
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
 * Main system loop.
 */
static void system_loop(){
	static int i = 0;
	static int j = 0;
	while(1){
		//SLEEP200
		
		//Sample test code to edit pixel buffer.
		if(i >= SCREEN_WIDTH - 1)
			i = 0;
		
		if(j >= SCREEN_HEIGHT - 1)
			j = 0;
	
		pixel_buffer[j][i]+= 0x1;
		
		i++;
		j++;
	}
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
	for(int r = 0; r < SCREEN_HEIGHT; r++){
		for(int c = 0; c < SCREEN_WIDTH; c++){
			pixel_buffer[r][c] = 0;
		}
	}
}

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

int main(void){
	//Init system using ASF.
	board_init();
	sysclk_init();
	
	//Init and start vga output.
	vga_init();
	vga_start();
	
	system_loop();
	
	vga_stop();
}