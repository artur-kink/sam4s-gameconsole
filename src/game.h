#ifndef _GAME_
#define _GAME_

#include "vga.h"
#include "controllers/controller.h"

#include "games/line_racer.h"

struct game_definition{
	char* name;
	unsigned char name_len;
	void* main_entry;	
};

unsigned char num_games;
struct game_definition* game_definitions;

void console_main(void);

inline void sleep_frame(void);
inline void sleep_frames(int count);
#endif