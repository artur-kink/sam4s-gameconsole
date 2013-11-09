#include <compiler.h>
#include <status_codes.h>

// From module: GPIO - General purpose Input/Output
#include <gpio.h>
// From module: Generic board support
#include <board.h>
// From module: IOPORT - General purpose I/O service
#include <ioport.h>
// From module: Interrupt management - SAM implementation
#include <interrupt.h>
// From module: PIO - Parallel Input/Output Controller
#include <pio.h>
// From module: PMC - Power Management Controller
#include <pmc.h>
#include <sleep.h>
// From module: Part identification macros
#include <parts.h>
// From module: SAM4S startup code
#include <exceptions.h>
// From module: System Clock Control - SAM4S implementation
#include <sysclk.h>
// From module: WDT - Watchdog Timer
#include <wdt.h>
// From module: pio_handler support enabled
#include <pio_handler.h>

#include <tc.h>

unsigned long active = LED_0_ACTIVE;
int counter = 0;
void TC_Handler(void){
	tc_disable_interrupt(TC0, 0, TC_IER_CPAS);
	if((tc_get_status(TC0, 0) & TC_IER_CPAS) == TC_IER_CPAS){
		tc_stop(TC0, 0);
		counter = 0;
		active = !active;
		ioport_set_pin_level(LED_0_PIN, active);
		tc_start(TC0, 0);
	}
	tc_enable_interrupt(TC0, 0, TC_IER_CPAS);
	return;
}

int main(void){
	board_init();
	
	NVIC_DisableIRQ(TC0_IRQn);
	NVIC_ClearPendingIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn, 0);
	NVIC_EnableIRQ(TC0_IRQn);
	
	sysclk_enable_peripheral_clock(ID_TC0);
	
	//Initialize counter.
	tc_init(TC0, 0, TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_WAVE | TC_CMR_ACPC_CLEAR);
	tc_set_writeprotect(TC0, 0);
	tc_write_ra(TC0, 0, 60000);
	tc_set_writeprotect(TC0, 1);
	tc_disable_interrupt(TC0, 0, 0xFFFFFFFF);
	tc_enable_interrupt(TC0, 0, TC_IER_CPAS);
	//Enable clock.
	tc_start(TC0, 0);
	
	while (1) {
		asm("nop");
	}
}
