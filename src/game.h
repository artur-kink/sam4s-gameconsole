#ifndef _GAME_
#define _GAME_

#include "vga.h"

void game_main(void);

inline void sleep_frame(void){
	while(hsync_counter != 0){
		SLEEP1
	}
}

#endif