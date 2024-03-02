#ifndef stardust_include_file
#define stardust_include_file

#include "flight.h"

void stardust_Draw();
void stardust_Move(enum viewDirMode_t viewDirMode,
		unsigned char player_speed, signed char player_pitch, signed char player_roll);

#endif
