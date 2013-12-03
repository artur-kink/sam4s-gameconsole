#include "game.h"

#include "controllers/ps_controller.h"
#include "games/line_racer.h"

short p1_x;
short p1_y;
short p1_ydir;
short p1_xdir;

short p2_x;
short p2_y;
short p2_ydir;
short p2_xdir;

unsigned char p1_score;
unsigned char p2_score;

char game_state;
char menu_drawn;

void game_setup(void){
	
	if(p1_score == 10 || p2_score == 10){
		game_state = 2;
		menu_drawn= 0;
	}
	
	p1_x = 30;
	p1_y = SCREEN_HEIGHT/2;
	p1_ydir = 0;
	p1_xdir = 1;
	
	p2_x = SCREEN_WIDTH - 30;
	p2_y = SCREEN_HEIGHT/2;
	p2_ydir = 0;
	p2_xdir = -1;
	
	vga_clear_screen(COLOR_BLACK);
	vga_draw_string(20, 15, COLOR_WHITE, "Line Racer Score:", 17);
	vga_draw_character(156, 15, COLOR_RED, p1_score + 48);
	vga_draw_character(164, 15, COLOR_WHITE, '/');
	vga_draw_character(172, 15, COLOR_YELLOW, p2_score + 48);
	vga_draw_line(0, 28, SCREEN_WIDTH, 28, COLOR_WHITE);
	vga_draw_line(0, 28, 0, SCREEN_HEIGHT, COLOR_WHITE);
	vga_draw_line(SCREEN_WIDTH-1, 28, SCREEN_WIDTH-1, SCREEN_HEIGHT, COLOR_WHITE);
	vga_draw_line(0, SCREEN_HEIGHT-1, SCREEN_WIDTH, SCREEN_HEIGHT-1, COLOR_WHITE);
}

void line_racer_main(void){
	
	game_state = 0;
	p1_score = p2_score = 0;
	menu_drawn = 0;
	char screen_flash = 0;
	while(1){
		
		if(game_state == 0){
			screen_flash++;
			vga_draw_string(SCREEN_WIDTH/2 - 37, SCREEN_HEIGHT/2 - 10, screen_flash, "LINE RACER", 10);
			vga_draw_string(SCREEN_WIDTH/2 - 40, SCREEN_HEIGHT/2, screen_flash, "Press Start", 11);
			
			if(menu_drawn == 0){
				vga_clear_screen(COLOR_BLACK);
				menu_drawn = 1;
				sleep_frames(45);
			}else if(controller_is_down(0, ps_start)){
				game_setup();
				p1_score = p2_score = 0;
				game_state = 1;
				sleep_frames(45);
			}
			
		}else if(game_state == 1){
			if(controller_is_down(0, ps_start)){
				game_state = 0;
				menu_drawn = 0;
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
		
			if(vga_get_pixel(p1_x, p1_y) != COLOR_BLACK){
				//Check for tie.
				if(vga_get_pixel(p2_x, p2_y) != COLOR_BLACK){
					game_setup();
				}else{
					p2_score++;
					game_setup();
				}
			}else if(vga_get_pixel(p2_x, p2_y) != COLOR_BLACK){
				p1_score++;
				game_setup();
			}
		
			vga_set_pixel(p1_x, p1_y, COLOR_RED);
			vga_set_pixel(p2_x, p2_y, COLOR_YELLOW);
		}else{
			if(menu_drawn == 0){
				if(p1_score == 10){
					vga_draw_string(SCREEN_WIDTH/2 - 32, SCREEN_HEIGHT/2, COLOR_RED, "RED WINS", 8);
				}else{
					vga_draw_string(SCREEN_WIDTH/2 - 44, SCREEN_HEIGHT/2, COLOR_YELLOW, "YELLOW WINS", 11);
				}
				
				menu_drawn = 1;
				sleep_frames(60);
			}else if(controller_is_down(0, ps_start)){
				game_state = 0;
				menu_drawn = 0;
			}
			
		}
		
		sleep_frame();
	}
}