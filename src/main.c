#include "asf.h"
#include "vga.h"


/** 
 * Main system loop.
 */
void system_loop(){
	static int i = 0;
	while(1){
		i++;
		vga_clear_screen(i);
	}
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
	return 0;
}