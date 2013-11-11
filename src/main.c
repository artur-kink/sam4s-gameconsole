#include "asf.h"


//VGA Info
//640x480 60hz
//Field Rate: 60.04hz
//System Clock: 120mhz - 120,000,000hz


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
#define PIN_RED EXT3_PIN_17

//Debug LED state.
unsigned long active = LED_0_ACTIVE;

/** 
 * Interrupt handler for VGA sync signals.
 * Currently includes debug counters.
 */
void TC_VGA_Handler(void){
	static unsigned int sec_counter = 0;
	//NVIC_ClearPendingIRQ(TC0_IRQn);
	
	
	if(tc_get_status(TC_VGA, TCC_VSYNC_PULSE) & TC_IER_CPCS){
		tc_stop(TC_VGA, TCC_VSYNC_PULSE);
		
		ioport_set_pin_level(PIN_VSYNC, 0);
		
		//Debug 1 second led counter.
		sec_counter++;
		if(sec_counter == 60){
			active = !active;
			ioport_set_pin_level(LED_0_PIN, active);
			sec_counter = 0;
		}
		tc_start(TC_VGA, TCC_HSYNC);
		ioport_set_pin_level(PIN_RED, 1);
	}else if(tc_get_status(TC_VGA, TCC_VSYNC) & TC_IER_CPCS){
		tc_start(TC_VGA, TCC_VSYNC_PULSE);
		tc_stop(TC_VGA, TCC_HSYNC);
		ioport_set_pin_level(PIN_VSYNC, 1);
		ioport_set_pin_level(PIN_RED, 0);
	}else if(tc_get_status(TC_VGA, TCC_HSYNC) & TC_IER_CPCS){
		ioport_set_pin_level(PIN_HSYNC, 1);
		ioport_set_pin_level(PIN_HSYNC, 0);
	}
	
	//ioport_set_pin_level(PIN_RED, 1);
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
	tc_init(TC_VGA, TCC_VSYNC, TC_CMR_TCCLKS_TIMER_CLOCK4 | TC_CMR_CPCTRG);
	
	//Set Compare register value to fire interrupt every 1/60 seconds.
	tc_write_rc(TC_VGA, TCC_VSYNC, 15625);
	
	//Enable compare interrupt.
	tc_enable_interrupt(TC_VGA, TCC_VSYNC, TC_IER_CPCS);
	
	//Initialize vsync pulse length timer.
	tc_set_writeprotect(TC_VGA, 0);
	tc_init(TC_VGA, TCC_VSYNC_PULSE, TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_CPCTRG);
	//Set Compare register value to fire interrupt every 2/60/524 seconds(The length of 2 lines = length of vsync pulse).
	tc_write_rc(TC_VGA, TCC_VSYNC_PULSE, 3817);
	
	//Enable compare interrupt.
	tc_enable_interrupt(TC_VGA, TCC_VSYNC_PULSE, TC_IER_CPCS);
	
	//Setup vsync pin
	ioport_enable_pin(PIN_VSYNC);
	ioport_set_pin_dir(PIN_VSYNC, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_VSYNC, 0);
	
	//Initialize hsync timer.
	tc_set_writeprotect(TC_VGA, 0);
	tc_init(TC_VGA, TCC_HSYNC, TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_CPCTRG);
	//Set Compare register value to fire interrupt every 2/60/524 seconds(The length of 2 lines = length of vsync pulse).
	tc_write_rc(TC_VGA, TCC_HSYNC, 1908);
	
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

	//Start vsync timer.
	tc_start(TC_VGA, TCC_VSYNC);
	
	//Enable IRQ for TC0
	NVIC_ClearPendingIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn, 0);
	NVIC_EnableIRQ(TC0_IRQn);
	
	//Main loop
	while (1) {
		asm("nop");
	}
}
