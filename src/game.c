#include "game.h"

#include "controllers/ps_controller.h"

#include "games/line_racer.h"
#include "games/pong.h"
#include "games/256_demo.h"

void console_main(void){
	
	//Init games
	num_games = 3;
	game_definitions = malloc(sizeof(struct game_definition)*num_games);
	game_definitions[0].main_entry = &line_racer_main;
	game_definitions[0].name = "Line Racer";
	game_definitions[0].name_len = 10;
	
	game_definitions[1].main_entry = &pong_main;
	game_definitions[1].name = "Pong";
	game_definitions[1].name_len = 4;
	
	game_definitions[2].main_entry = &_256_main;
	game_definitions[2].name = "256 Color Demo";
	game_definitions[2].name_len = 14;
	
	unsigned char selection = 0;
	while(1){
		vga_draw_string(20, 20, COLOR_WHITE, "Select Game", 11);
		int i = 0;
		for(i = 0; i < num_games; i++){
			if(selection == i){
				vga_draw_character(29, 30 + i*8, COLOR_RED, '>');
				vga_draw_string(35, 30 + i*8, COLOR_RED, game_definitions[i].name, game_definitions[i].name_len);
			}else{
				vga_draw_character(29, 30 + i*8, COLOR_BLACK, '>');
				vga_draw_string(35, 30 + i*8, COLOR_WHITE, game_definitions[i].name, game_definitions[i].name_len);
			}
			
		}
		
		if(controller_is_down(0, ps_start)){
			void (*game_entry) ();
			game_entry = game_definitions[selection].main_entry;
			game_entry();
			vga_clear_screen(COLOR_BLACK);
		}else if(controller_is_down(0, ps_dpad_down)){
			selection++;
			if(selection >= num_games)
				selection = 0;
			sleep_frames(25);
		}else if(controller_is_down(0, ps_dpad_up)){
			selection--;
			if(selection >= num_games)
				selection = num_games-1;
			sleep_frames(25);
		}
		
		sleep_frame();
	}
	
}

void sleep_frame(void){
	unsigned int sleep_frame_val = vga_get_sec_frame();
	while(1){
		if(sleep_frame_val != vga_get_sec_frame())
			break;
	}
}

void sleep_frames(int count){
	unsigned int sleep_frames_count = count;
	while(sleep_frames_count > 0){
		sleep_frame();
		sleep_frames_count--;
	}
}