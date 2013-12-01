#include "game.h"

void game_main(void){
	int p1_x = 250;
	int p1_y = 15;
	int p1_ydir = 1;
	while(1){
		if(p1_ydir == 1 && p1_y >= (SCREEN_HEIGHT-1)){
			p1_ydir = -1;
		}else if(p1_ydir == -1 && p1_y <= 1){
			p1_ydir = 1;
		}
		
		vga_set_pixel(p1_x, p1_y, COLOR_BLACK);
		
		if(p1_ydir)
			p1_y += p1_ydir;
		SLEEP200
		vga_set_pixel(p1_x, p1_y, COLOR_RED);
		sleep_frame();
	}
}