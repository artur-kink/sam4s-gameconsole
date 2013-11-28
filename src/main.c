#include "asf.h"
#include "vga.h"


/** 
 * Main system loop.
 */
static void system_loop(){
	static int i = 0;
	static int j = 0;
	while(1){
		//SLEEP200
		
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
	
	//Init and start vga output.
	vga_init();
	vga_start();
	
	system_loop();
	
	vga_stop();
	return 0;
}