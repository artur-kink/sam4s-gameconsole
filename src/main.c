#include "asf.h"
#include "game.h"

int main(void){
	//Init system using ASF.
	board_init();
	sysclk_init();
	
	controller_init();
	
	//Init and start vga output.
	vga_init();
	vga_start();
	
	console_main();
	
	vga_stop();
	return 0;
}