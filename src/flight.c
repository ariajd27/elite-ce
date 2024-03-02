#include <graphx.h>
#include <keypadc.h>

#include "gfx/gfx.h"

#include "xorgfx.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "variables.h"

#include "ship.h"
#include "ship_data.h"
#include "stardust.h"
#include "generation.h"
#include "market.h"
#include "flight.h"
#include "input.h"

#include <debug.h>

unsigned char player_speed;
signed char player_acceleration;
signed char player_roll;
signed char player_pitch;

enum viewDirMode_t viewDirMode;
enum player_condition_t player_condition;

void resetPlayerCondition()
{
	player_condition = GREEN;
}

void flightInit()
{
	player_speed = 0;
	player_acceleration = 0;
	player_roll = 0;
	player_pitch = 0;

	viewDirMode = FRONT;
}

void drawSpaceView()
{
	if (viewDirMode == FRONT) 
		xor_CenterText("Front View", 10, 10);
	else if (viewDirMode == LEFT)
		xor_CenterText("Left View", 9, 10);
	else if (viewDirMode == RIGHT)
		xor_CenterText("Right View", 10, 10);
	else xor_CenterText("Rear View", 9, 10);

	xor_Crosshair(VIEW_HCENTER, VIEW_VCENTER, CRS_SPREAD, CRS_SIZE);

	// ships
	FlipAxes(viewDirMode);
	for(unsigned char i = 0; i < numShips; i++) DrawShip(i);
	RestoreAxes(viewDirMode);

	stardust_Move(viewDirMode, player_speed, player_pitch, player_roll);
	stardust_Draw(); // order is not really important here
}

void drawDashboard()
{
	// sprited sections
	gfx_Sprite(dashleft, DASH_HOFFSET, DASH_VOFFSET);
	gfx_Sprite(dashcenter, DASH_HOFFSET_CENTER, DASH_VOFFSET);
	gfx_Sprite(dashright, DASH_HOFFSET_RIGHT, DASH_VOFFSET);

	// radar dots
	for (unsigned char i = 0; i < numShips; i++)
	{
		// check to make sure ship in range
		if (ships[i].position.x < -63 * 256) continue;
		if (ships[i].position.x > 63 * 256) continue;
		if (ships[i].position.y < -63 * 256) continue;
		if (ships[i].position.y > 63 * 256) continue;
		if (ships[i].position.z < -63 * 256) continue;
		if (ships[i].position.z > 63 * 256) continue;

		unsigned int const x = RADAR_HCENTER + ships[i].position.x / RADAR_XSCALE;
		unsigned char const y = RADAR_VCENTER - ships[i].position.z / RADAR_ZSCALE;
		signed char dy = ships[i].position.y / RADAR_YSCALE;

		// clip to fit in center dash segment
		if (y + dy < DASH_VOFFSET + 2)
		{
			dy += (DASH_VOFFSET + 3) - (y + dy);
		}
		else if (y + dy > GFX_LCD_HEIGHT - 2)
		{
			dy -= (y + dy) - (GFX_LCD_HEIGHT - 3);
		}

		gfx_SetColor(COLOR_YELLOW);
		gfx_Line(x, y, x, y + dy);
		gfx_SetPixel(x, y + dy);
		gfx_SetPixel(x - 1, y + dy);
	}

	// right panel
	// speed display
	gfx_SetColor(COLOR_YELLOW);
	unsigned char const speedBarLength = 32 * player_speed / PLAYER_MAX_SPEED;
	gfx_FillRectangle(DASH_HOFFSET_RIGHT + 4, DASH_VOFFSET + 2, speedBarLength, 3);

	// roll display
	signed char rollBarOffset = player_roll / 2;
	if (rollBarOffset == 16) rollBarOffset--;
	else if (rollBarOffset == -16) rollBarOffset++;
	gfx_FillRectangle(DASH_HOFFSET_RIGHT + 20 + rollBarOffset, DASH_VOFFSET + 10, 2, 3);

	// pitch display
	signed char pitchBarOffset = player_pitch * 2;
	if (pitchBarOffset == 16) pitchBarOffset--;
	else if (pitchBarOffset == -16) pitchBarOffset++;
	gfx_FillRectangle(DASH_HOFFSET_RIGHT + 20 + pitchBarOffset, DASH_VOFFSET + 18, 2, 3);
}

bool doFlightInput()
{
	dbg_printf("doing flight input...\n");

	updateKeys();

	// acceleration
	if (kb_IsDown(kb_Key2nd)) player_acceleration = 3;
	else if (kb_IsDown(kb_KeyAlpha)) player_acceleration = -1;
	
	if (!yequ) // pitch/roll controls
	{
		// roll
		if (left) player_roll -= 4;
		else if (right) player_roll += 4;

		// damping
		else if (-1 <= player_roll && player_roll <= 1) player_roll = 0; 
		else if (player_roll < 0) player_roll += 2;
		else if (player_roll > 0) player_roll -= 2;

		// clamping
		if (player_roll > 31) player_roll = 31;
		else if (player_roll < -31) player_roll = -31;
	
		// pitch
		if (up) player_pitch++;
		else if (down) player_pitch--;

		// damping
		else if (player_pitch < 0) player_pitch++;
		else if (player_pitch > 0) player_pitch--;

		// clamping
		if (player_pitch > 7) player_pitch = 7;
		else if (player_pitch < -7) player_pitch = -7;
	}
	else // view switching
	{
		if (up) viewDirMode = FRONT;
		else if (left) viewDirMode = LEFT;
		else if (right) viewDirMode = RIGHT;
		else if (down) viewDirMode = REAR;

		// still need to do pitch/roll damping
		if (player_pitch < 0) player_pitch++;
		else if (player_pitch > 0) player_pitch--;
		if (player_roll < 0) player_roll++;
		else if (player_roll > 0) player_roll--;	
	}

	updatePrevKeys();

	if (graph && prevGraph == 1) // 1 bc we are after 1 update... kinda janky, but...
	{
		currentMenu = MAIN;
		return false;
	}
	else return true;
}

void doFlight()
{
	while (doFlightInput())
	{
		clock_t frameTimer = clock();

		// black background
		// gfx_SetColor(COLOR_BLACK);
		// gfx_FillRectangle(0, 0, GFX_LCD_WIDTH, GFX_LCD_HEIGHT);	
		gfx_FillScreen(COLOR_BLACK);

		// outer white frame
		gfx_SetColor(COLOR_WHITE);
		gfx_Rectangle(DASH_HOFFSET, 0, GFX_LCD_WIDTH - 2 * DASH_HOFFSET, GFX_LCD_HEIGHT - dashleft_height);

		// apply acceleration to speed
		if (player_acceleration >= 0 || player_speed >= -1 * player_acceleration) 
		{
			player_speed += player_acceleration;
		}
		if (player_speed > PLAYER_MAX_SPEED)
		{
			player_speed = PLAYER_MAX_SPEED;
		}
		player_acceleration = 0;
	
		// tidy vectors for each ship -- one ship per cycle
		ships[drawCycle % MAX_SHIPS].orientation = orthonormalize(ships[drawCycle % MAX_SHIPS].orientation);

		// do ai -- two ships per cycle
		DoAI(drawCycle % MAX_SHIPS);
		DoAI((drawCycle + MAX_SHIPS / 2) % MAX_SHIPS);
	
		for (unsigned char i = 0; i < numShips; i++)
		{
			// apply speed to other ships
			ships[i].position.z -= player_speed;
	
			// apply pitch and roll to other ships' positions
			signed int oldY = ships[i].position.y - (player_roll * ships[i].position.x) / 256;
			ships[i].position.z += (player_pitch * oldY) / 256;
			ships[i].position.y = oldY - (player_pitch * ships[i].position.z) / 256;
			ships[i].position.x += (player_roll * ships[i].position.y) / 256;
	
			// apply pitch and roll to other ships' orientations
			for (unsigned char j = 0; j < 9; j += 3)
			{
				ships[i].orientation.a[j + 1] -= player_roll * ships[i].orientation.a[j + 0] / 256;
				ships[i].orientation.a[j + 0] += player_roll * ships[i].orientation.a[j + 1] / 256;
				ships[i].orientation.a[j + 1] -= player_pitch * ships[i].orientation.a[j + 2] / 256;
				ships[i].orientation.a[j + 2] += player_pitch * ships[i].orientation.a[j + 1] / 256;
			}
	
			MoveShip(i);
		}

		drawSpaceView();
		drawCycle++;

		drawDashboard();

		while (clock() - frameTimer < FRAME_TIME);

		gfx_BlitBuffer();
	}
}
