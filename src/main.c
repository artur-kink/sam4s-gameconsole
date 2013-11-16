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

//RGB port, The port where the rgb pins are.
static Pio* rgb_port;

//Red Pin
#define PIN_RED EXT2_PIN_3
//Green Pin
#define PIN_GREEN EXT2_PIN_4
//Blue Pin
#define PIN_BLUE EXT3_PIN_13

//Bitmask for Red
#define COLOR_RED 1
//Bitmask for Green
#define COLOR_GREEN 2
//Bitmask for Blue
#define COLOR_BLUE 4

//Bitmasks for additive colors.
#define COLOR_MAGENTA (COLOR_RED + COLOR_BLUE)
#define COLOR_TEAL (COLOR_GREEN + COLOR_BLUE)
#define COLOR_YELLOW (COLOR_RED + COLOR_GREEN)
#define COLOR_WHITE (COLOR_RED + COLOR_GREEN + COLOR_BLUE)
#define COLOR_BLACK 0

//Height of screen, the number of lines.
#define SCREEN_HEIGHT 180
//Width of screen, number of pixels per line.
#define SCREEN_WIDTH 266

//Screen pixel buffer.
unsigned short pixel_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];

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
		
		//Start hsync
		hsync_counter = 0;
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
	ioport_set_pin_level(PIN_HSYNC, 0);
	SLEEP200
	SLEEP200
	SLEEP20
	SLEEP20
	SLEEP10
	ioport_set_pin_level(PIN_HSYNC, 1);
	
	//Blit pixel buffer.
	int line = hsync_counter/4;
	for(int i = 0; i < 200; i++){
		//Blit pixel colors.
		unsigned char pixel = pixel_buffer[line][i];
		rgb_port->PIO_ODSR = pixel;
		SLEEP5
		SLEEP1 SLEEP1
	}
	
	//Disable any colors in use.
	ioport_set_pin_level(PIN_RED, 0);
	ioport_set_pin_level(PIN_GREEN, 0);
	ioport_set_pin_level(PIN_BLUE, 0);
	
	//If we have drawn 524+ lines, disable hsync timer.
	hsync_counter++;
	if(hsync_counter >= 524){
		tc_stop(TC_VGA, TCC_HSYNC);
	}
	
	return;
}

/** 
 * Main system loop.
 */
static void system_loop(){
	static int i = 0;
	static int j = 0;
	while(1){
		SLEEP200
		
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

int main(void){
	//Init system using ASF.
	board_init();
	sysclk_init();
	
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

	//Get RGB port, all rgb pins should be on the same port.
	rgb_port = arch_ioport_pin_to_base(PIN_RED);

	//Setup red pin
	ioport_enable_pin(PIN_RED);
	ioport_set_pin_dir(PIN_RED, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_RED, 0);
	
	
	//Setup green pin
	ioport_enable_pin(PIN_GREEN);
	ioport_set_pin_dir(PIN_GREEN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_GREEN, 0);

	//Setup blue pin
	ioport_enable_pin(PIN_BLUE);
	ioport_set_pin_dir(PIN_BLUE, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_BLUE, 0);

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
	
	//Start vsync timer.
	tc_start(TC_VGA, TCC_VSYNC);
	
	system_loop();
}