#include "stardust.h"
#include "xorgfx.h"
#include "flight.h"

#include "variables.h"

struct stardust_t stardust[STARDUST_COUNT];

void stardust_Draw()
{
	for(unsigned char i = 0; i < STARDUST_COUNT; i++)
	{
		signed int const x = (signed int)VIEW_HCENTER + stardust[i].x / 256;
		signed int const y = (signed int)VIEW_VCENTER + stardust[i].y / 256;

		if (!xor_InClipRegion(x, y))
		{
			stardust[i].active = false;
			continue;
		}

		if (stardust[i].z < 64 * 256) xor_FillRectangle(x, y, 2, 2);
		else if (stardust[i].z < 128 * 256) xor_FillRectangle(x, y, 2, 1);
		else xor_FillRectangle(x, y, 1, 1);
	}
}

void stardust_Move(enum viewDirMode_t viewDirMode, 
		unsigned char player_speed, signed char player_pitch, signed char player_roll)
{
	for (unsigned char i = 0; i < STARDUST_COUNT; i++)
	{
		if (!stardust[i].active)
		{
			if (viewDirMode == FRONT || viewDirMode == REAR)
			{
				stardust[i].x = (signed int)(rand() % (DASH_WIDTH * 256)) 
					- (signed int)(DASH_WIDTH * 128);
				stardust[i].y = (signed int)(rand() % (DASH_VOFFSET * 256)) 
					- (signed int)(DASH_VOFFSET * 128);

				stardust[i].z = rand() % 32768;
				if (viewDirMode == FRONT)
				{
					stardust[i].z += 32768;
				}
			}
			else
			{
				if (viewDirMode == LEFT)
				{
					stardust[i].x = 128 * DASH_WIDTH;
				}
				else
				{
					stardust[i].x = -128 * DASH_WIDTH;
				}
				stardust[i].y = (signed int)(rand() % (DASH_VOFFSET * 256)) 
					- (signed int)(DASH_VOFFSET * 128);
				stardust[i].z = rand() % 65536;
			}

			stardust[i].active = true;
		}

		// check to see if we fly past it this frame
		if (player_speed * 64 >= stardust[i].z)
		{
			stardust[i].active = false;
			continue;
		}

		// avoid them all getting 10 million miles behind us in rear view
		if (viewDirMode == REAR && stardust[i].z >= 32768)
		{
			stardust[i].active = false;
			continue;
		}

		unsigned char const q = 64 * player_speed / (stardust[i].z / 256);

		// apply player speed
		if (viewDirMode == FRONT)
		{
			stardust[i].z -= player_speed * 64;
			stardust[i].y += q * stardust[i].y / 256;
			stardust[i].x += q * stardust[i].x / 256;
		}
		else if (viewDirMode == REAR)
		{
			stardust[i].z += player_speed * 64;
			stardust[i].y -= q * stardust[i].y / 256;
			stardust[i].x -= q * stardust[i].x / 256;
		}
		else if (viewDirMode == LEFT)
			stardust[i].x -= 128 * q;
		else stardust[i].x += 128 * q;

		// apply roll (like pitch in the side views)
		if (viewDirMode == FRONT)
		{
			stardust[i].y -= player_roll * stardust[i].x / 256;
			stardust[i].x += player_roll * stardust[i].y / 256;
		}
		else if (viewDirMode == REAR)
		{
			stardust[i].y += player_roll * stardust[i].x / 256;
			stardust[i].x -= player_roll * stardust[i].y / 256;
		}
		else if (viewDirMode == LEFT)
			stardust[i].y += player_roll * 256;
		else stardust[i].y -= player_roll * 256;

		// apply pitch (like roll in the side views)
		if (viewDirMode == FRONT) stardust[i].y -= player_pitch * 256;
		else if (viewDirMode == REAR) stardust[i].y += player_pitch * 256;
		else if (viewDirMode == LEFT)
		{
			stardust[i].y -= player_pitch * stardust[i].x / 256;
			stardust[i].x += player_pitch * stardust[i].y / 256;
		}
		else
		{
			stardust[i].y += player_pitch * stardust[i].x / 256;
			stardust[i].x -= player_pitch * stardust[i].y / 256;
		}
		// stardust[i].x += 2 * (player_pitch * stardust[i].y / 256) * (player_pitch * stardust[i].y / 256);
	}
}
