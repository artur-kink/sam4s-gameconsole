#include <compiler.h>
#include <status_codes.h>
#include <gpio.h>
#include <board.h>
#include <ioport.h>
#include <interrupt.h>
#include <pmc.h>
#include <sleep.h>
#include <parts.h>
#include <exceptions.h>
#include <sysclk.h>
#include <tc.h>

//VGA Info
//640x480 60hz
//Field Rate: 60.04hz
//System Clock: 120mhz - 120,000,000hz


//VGA Timer
#define TC_VGA TC0
//Horizontal Sync Channel
#define TCC_HORIZONTAL 0

//VSync Pin
#define PIN_VSYNC EXT3_PIN_3
//HSync Pin
#define PIN_HSYNC EXT3_PIN_5

#define PIN_RED EXT3_PIN_17

//Debug LED state.
unsigned long active = LED_0_ACTIVE;

/** 
 * Interrupt handler for VGA sync signals.
 * Currently includes debug counters.
 */
void TC_VGA_Handler(void){
	static int counter = 0;
	static unsigned int sec_counter = 0;
	
	if(tc_get_status(TC_VGA, TCC_HORIZONTAL) & TC_IER_CPCS){
		tc_disable_interrupt(TC_VGA, TCC_HORIZONTAL, TC_IER_CPCS);
		
		ioport_set_pin_level(PIN_RED, 0);
		
		//Enable vsync for 2 lines if we have drawn 524 lines.
		if(counter == 0){
			ioport_set_pin_level(PIN_VSYNC, 1);
		}else if(counter == 2){
			//Debug 1 second led counter.
			sec_counter++;
			if(sec_counter == 60){
				active = !active;
				ioport_set_pin_level(LED_0_PIN, active);
				sec_counter = 0;
			}
			ioport_set_pin_level(PIN_VSYNC, 0);
			
		}else{
			hsync:
			ioport_set_pin_level(PIN_HSYNC, 1);
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
			ioport_set_pin_level(PIN_HSYNC, 0);
			if(counter == 526){
				counter = -1;
			}
		}
		counter++;
		ioport_set_pin_level(PIN_RED, 1);
		tc_enable_interrupt(TC_VGA, TCC_HORIZONTAL, TC_IER_CPCS);
	}
	
	return;
}

int main(void){
	board_init();
	sysclk_init();
	//Enable IRQ for TC0
	NVIC_DisableIRQ(TC0_IRQn);
	NVIC_ClearPendingIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn, 0);
	NVIC_EnableIRQ(TC0_IRQn);
	
	sysclk_enable_peripheral_clock(ID_TC0);
	
	//Initialize hsync clock clock.
	tc_init(TC_VGA, TCC_HORIZONTAL, TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_CPCTRG);
	
	//Set Compare register value to fire interrupt every 1/60/524 seconds.
	tc_set_writeprotect(TC_VGA, 0);
	tc_write_rc(TC_VGA, TCC_HORIZONTAL, 1907);
	tc_set_writeprotect(TC_VGA, 1);
	
	//Disable all but compare interrupt.
	tc_disable_interrupt(TC_VGA, TCC_HORIZONTAL, 0xFFFFFFFF);
	tc_enable_interrupt(TC_VGA, TCC_HORIZONTAL, TC_IER_CPCS);
	
	//Setup vsync pin
	ioport_enable_pin(PIN_VSYNC);
	ioport_set_pin_dir(PIN_VSYNC, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_VSYNC, 0);
	
	//Setup hsync pin
	ioport_enable_pin(PIN_HSYNC);
	ioport_set_pin_dir(PIN_HSYNC, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_HSYNC, 0);
	
	//Setup hsync pin
	ioport_enable_pin(PIN_RED);
	ioport_set_pin_dir(PIN_RED, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIN_RED, 0);
	
	unsigned int hz = sysclk_get_main_hz();
	hz = sysclk_get_peripheral_hz();
	
	//Start timer.
	tc_start(TC_VGA, TCC_HORIZONTAL);
	
	//Main loop
	while (1) {
		asm("nop");
	}
}
