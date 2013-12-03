#include "game.h"

#include "games/line_racer.h"



void console_main(void){
	line_racer_main();
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