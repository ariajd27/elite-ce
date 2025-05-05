#ifndef stardust_include_file
#define stardust_include_file

#include "flight.h"
#include "variables.h"

extern struct stardust_t {
	bool active;
	signed int x;
	signed int y;
	signed int z;
} stardust[STARDUST_COUNT];

void stardust_Draw();
void stardust_Move(enum viewDirMode_t viewDirMode,
		unsigned char player_speed, signed char player_pitch, signed char player_roll);

#endif
