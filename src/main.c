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

//VGA Timer
#define TC_VGA TC0
//Vertical Sync Channel
#define TCC_VERTICAL 0

//Debug LED state.
unsigned long active = LED_0_ACTIVE;

/** 
 * Interrupt handler for VGA sync signals.
 * Currently includes debug counters.
 */
void TC_VGA_Handler(void){
	static int counter = 0;
	if(tc_get_status(TC_VGA, TCC_VERTICAL) & TC_IER_CPAS){
		tc_disable_interrupt(TC_VGA, TCC_VERTICAL, TC_IER_CPAS);
		tc_stop(TC_VGA, TCC_VERTICAL);
		counter++;
		if(counter > 10){
			counter = 0;
			active = !active;
			ioport_set_pin_level(LED_0_PIN, active);
		}
		tc_start(TC_VGA, TCC_VERTICAL);
		tc_enable_interrupt(TC_VGA, TCC_VERTICAL, TC_IER_CPAS);
	}
	return;
}

int main(void){
	board_init();
	
	//Enable IRQ for TC0
	NVIC_DisableIRQ(TC0_IRQn);
	NVIC_ClearPendingIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn, 0);
	NVIC_EnableIRQ(TC0_IRQn);
	
	sysclk_enable_peripheral_clock(ID_TC0);
	
	//Initialize counter.
	tc_init(TC_VGA, TCC_VERTICAL, TC_CMR_TCCLKS_TIMER_CLOCK4 | TC_CMR_WAVE | TC_CMR_ACPC_CLEAR);
	tc_set_writeprotect(TC_VGA, 0);
	tc_write_ra(TC_VGA, TCC_VERTICAL, 29297);
	tc_set_writeprotect(TC_VGA, 1);
	tc_disable_interrupt(TC_VGA, TCC_VERTICAL, 0xFFFFFFFF);
	tc_enable_interrupt(TC_VGA, TCC_VERTICAL, TC_IER_CPAS);
	//Enable clock.
	tc_start(TC_VGA, TCC_VERTICAL);
	
	//Main loop
	while (1) {
		asm("nop");
	}
}
