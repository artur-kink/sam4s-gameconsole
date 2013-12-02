#include "game.h"

#include "controllers/ps_controller.h"

int p1_x;
int p1_y;
int p1_ydir;
int p1_xdir;

int p2_x;
int p2_y;
int p2_ydir;
int p2_xdir;

void game_setup(void){
	p1_x = 30;
	p1_y = SCREEN_HEIGHT/2;
	p1_ydir = 0;
	p1_xdir = 1;
	
	p2_x = SCREEN_WIDTH - 30;
	p2_y = SCREEN_HEIGHT/2;
	p2_ydir = 0;
	p2_xdir = -1;
	
	vga_clear_screen(COLOR_BLACK);
}

void game_main(void){
	
	game_setup();
	
	while(1){
		
		if(controller_is_down(0, ps_start)){
			game_setup();
		}
		
		if(controller_is_down(0, ps_dpad_up) && p1_ydir != 1){
			p1_ydir = -1;
			p1_xdir = 0;
		}else if(controller_is_down(0, ps_dpad_down) && p1_ydir != -1){
			p1_ydir = 1;
			p1_xdir = 0;
		}else if(controller_is_down(0, ps_dpad_left) && p1_xdir != 1){
			p1_xdir = -1;
			p1_ydir = 0;
		}else if(controller_is_down(0, ps_dpad_right) && p1_xdir != -1){
			p1_xdir = 1;
			p1_ydir = 0;
		}
		
		if(p1_ydir == 1 && p1_y >= (SCREEN_HEIGHT-1)){
			p1_ydir = 0;
		}else if(p1_ydir == -1 && p1_y <= 1){
			p1_ydir = 0;
		}else if(p1_xdir == 1 && p1_x >= (SCREEN_WIDTH-1)){
			p1_xdir = 0;
		}else if(p1_xdir == -1 && p1_x <= 1){
			p1_xdir = 0;
		}

		p1_y += p1_ydir;
		p1_x += p1_xdir;
		
		vga_set_pixel(p1_x, p1_y, COLOR_RED);
		
		if(controller_is_down(0, ps_dpad_up) && p2_ydir != 1){
			p2_ydir = -1;
			p2_xdir = 0;
		}else if(controller_is_down(0, ps_dpad_down) && p2_ydir != -1){
			p2_ydir = 1;
			p2_xdir = 0;
		}else if(controller_is_down(0, ps_dpad_left) && p2_xdir != 1){
			p2_xdir = -1;
			p2_ydir = 0;
		}else if(controller_is_down(0, ps_dpad_right) && p2_xdir != -1){
			p2_xdir = 1;
			p2_ydir = 0;
		}
		
		if(p2_ydir == 1 && p2_y >= (SCREEN_HEIGHT-1)){
			p2_ydir = 0;
		}else if(p2_ydir == -1 && p2_y <= 1){
			p2_ydir = 0;
		}else if(p2_xdir == 1 && p2_x >= (SCREEN_WIDTH-1)){
			p2_xdir = 0;
		}else if(p2_xdir == -1 && p2_x <= 1){
			p2_xdir = 0;
		}

		p2_y += p2_ydir;
		p2_x += p2_xdir;
		
		vga_set_pixel(p2_x, p2_y, COLOR_RED);
		
		sleep_frame();
	}
}