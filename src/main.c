#include "asf.h"
#define _ASSERT_ENABLE_

//Sleep 5 cycles.
#define SLEEP5 asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
#define SLEEP10 SLEEP5 SLEEP5
#define SLEEP20 SLEEP10 SLEEP10
#define SLEEP50 SLEEP10 SLEEP10 SLEEP10 SLEEP10 SLEEP10
#define SLEEP100 SLEEP50 SLEEP50
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
//Red Pin
#define PIN_RED EXT3_PIN_9
//Green Pin
#define PIN_GREEN EXT3_PIN_13
//Blue Pin
#define PIN_BLUE EXT3_PIN_17

//Horizontal sync counter.
static int hsync_counter = 0;

//Debug LED state.
unsigned long active = LED_0_ACTIVE;
//Debug second counter.
static unsigned int sec_counter = 0;

void vsync_start(void);
void vsync_end(void);
void hsync(void);

/** 
 * Interrupt handler for vsync signal.
 */
void TC_VSYNC_Handler(void){
	
	if(tc_get_status(TC_VGA, TCC_VSYNC) & TC_IER_CPCS){
		vsync_start();
	}
	return;
}

/** 
 * Interrupt handler for vsync signal end.
 */
void TC_VSYNC_PULSE_Handler(void){
	if(tc_get_status(TC_VGA, TCC_VSYNC_PULSE) & TC_IER_CPCS){
		vsync_end();
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

void vsync_start(){
	//Start vsync pulse
	tc_start(TC_VGA, TCC_VSYNC_PULSE);
	ioport_set_pin_level(PIN_VSYNC, 0);
	
	ioport_set_pin_level(PIN_RED, 0);
	//Debug 1 second led counter.
	sec_counter++;
	if(sec_counter == 60){
		active = !active;
		ioport_set_pin_level(LED_0_PIN, active);
		sec_counter = 0;
	}
}

void vsync_end(){
	//Stop vsync pulse
	tc_stop(TC_VGA, TCC_VSYNC_PULSE);
	ioport_set_pin_level(PIN_VSYNC, 1);
	
	//Start hsync
	hsync_counter = 0;
	tc_start(TC_VGA, TCC_HSYNC);
	hsync();
}
static int is_red = 1;
void hsync(){
	ioport_set_pin_level(PIN_HSYNC, 0);
	
	SLEEP200
	SLEEP200
	SLEEP20
	SLEEP20
	SLEEP10
	
	ioport_set_pin_level(PIN_HSYNC, 1);
	
	for(int i = 0; i < 190; i++){
		is_red = !is_red;
		ioport_set_pin_level(PIN_RED, is_red);
	}
	//is_red = !is_red;
	ioport_set_pin_level(PIN_RED, 0);
	
	hsync_counter++;
	if(hsync_counter >= 524){
		tc_stop(TC_VGA, TCC_HSYNC);
	}
	
	return;
}

int main(void){
	board_init();
	sysclk_init();
	
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
	
	
	//Setup red pin
	ioport_enable_pin(PIN_RED);
	ioport_set_pin_dir(PIN_RED, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_RED, 0);

	//Enable IRQ
	NVIC_ClearPendingIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn, 0);
	NVIC_SetPriority(TC1_IRQn, 0);
	NVIC_SetPriority(TC2_IRQn, 0);
	NVIC_EnableIRQ(TC0_IRQn);
	NVIC_EnableIRQ(TC1_IRQn);
	NVIC_EnableIRQ(TC2_IRQn);
	
	//Start vsync timer.
	tc_start(TC_VGA, TCC_VSYNC);
	
	//Main loop
	while (1) {
		asm("nop");
	}
}

