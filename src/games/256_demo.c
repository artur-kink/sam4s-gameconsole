#include "256_demo.h"
#include "controllers/ps_controller.h"

void _256_main(void){
	unsigned char color = 0;
	while(!controller_is_down(0, ps_select)){
		vga_clear_screen(color);
		color++;
	}
}