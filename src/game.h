#ifndef _GAME_
#define _GAME_

#include "vga.h"
#include "controllers/controller.h"

void game_main(void);

inline void sleep_frame(void);
inline void sleep_frames(int count);
#endif