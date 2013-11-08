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

int main (void){
	board_init();

	while (1) {
		// Is button pressed?
		if (ioport_get_pin_level(BUTTON_0_PIN) == BUTTON_0_ACTIVE) {
			// Yes, so turn LED on.
			ioport_set_pin_level(LED_0_PIN, LED_0_ACTIVE);
		} else {
			// No, so turn LED off.
			ioport_set_pin_level(LED_0_PIN, !LED_0_ACTIVE);
		}
	}
}
