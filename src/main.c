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

void TC_Handler(void){
	active = !active;
	int x = tc_read_cv(TC0, 0);
	ioport_set_pin_level(LED_0_PIN, active);
	return;
}

int main(void){
	board_init();
	
	NVIC_DisableIRQ(TC0_IRQn);
	NVIC_ClearPendingIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn, 0);
	NVIC_EnableIRQ(TC0_IRQn);
	
	sysclk_enable_peripheral_clock(ID_TC0);
	
	tc_init (TC0, 0, TC_CCR_CLKEN);
	tc_enable_interrupt (TC0, 0, TC_IER_COVFS);
	//Enable clock.
	tc_start(TC0, 0);
	
	while (1) {
		int y = 0;
	}
}
