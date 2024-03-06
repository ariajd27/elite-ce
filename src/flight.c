#include <graphx.h>
#include <keypadc.h>

#include "gfx/gfx.h"

#include "xorgfx.h"

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "variables.h"
#include "linear.h"
#include "intmath.h"
#include "ship.h"
#include "ship_data.h"
#include "stardust.h"
#include "generation.h"
#include "flight.h"
#include "input.h"
#include "market.h"

#include <debug.h>

unsigned char player_speed = 0;
signed char player_roll = 0;
signed char player_pitch = 0;

enum viewDirMode_t viewDirMode = FRONT;
enum player_condition_t player_condition = DOCKED;

bool stationSoi = true;

void flt_DoLaunchAnimation()
{
	gfx_SetColor(COLOR_BLACK);
	gfx_FillRectangle(xor_clipX, xor_clipY, xor_clipWidth, xor_clipHeight);
	gfx_SetColor(COLOR_WHITE);
	gfx_Rectangle(xor_clipX, xor_clipY, xor_clipWidth, xor_clipHeight);
	gfx_BlitRectangle(gfx_buffer, xor_clipX, xor_clipY, xor_clipWidth, xor_clipHeight);

	clock_t timer;

	for (unsigned int inner = 8; inner < 16; inner++)
	{
		for (unsigned int current = inner; current <= 160; current <<= 1)
		{
			timer = clock();

			xor_SteppedCircle(VIEW_HCENTER, VIEW_VCENTER, current, 8);	
			gfx_BlitRectangle(gfx_buffer, xor_clipX, xor_clipY, xor_clipWidth, xor_clipHeight);

			while(clock() - timer < TUNNEL_FRAME_TIME);
		}
	}

	while (clock() - timer < TUNNEL_HOLD_TIME);

	// do the same stuff again to delete it
	for (unsigned int inner = 8; inner < 16; inner++)
	{
		for (unsigned int current = inner; current <= 160; current <<= 1)
		{
			timer = clock();

			xor_SteppedCircle(VIEW_HCENTER, VIEW_VCENTER, current, 8);	
			gfx_BlitRectangle(gfx_buffer, xor_clipX, xor_clipY, xor_clipWidth, xor_clipHeight);

			while(clock() - timer < TUNNEL_FRAME_TIME);
		}
	}
}

void flt_DoHyperspaceAnimation()
{
}

void launch()
{
	viewDirMode = FRONT;
	mkt_AdjustLegalStatus();

	player_speed = 12;

	// spawn station
	stationSoi = true;
	struct Ship* station = NewShip(BP_CORIOLIS,
								   (struct vector_t){ 0, 0, -256 },
								   Matrix(256,0,0, 0,256,0, 0,0,-256));
	station->roll = 127;

	// spawn planet
	NewShip(PLANET, (struct vector_t){ 0, 0, 49152 }, Matrix(256,0,0, 0,256,0, 0,0,256));

	flt_DoLaunchAnimation();
}

void resetPlayerCondition()
{
	if (player_condition == DOCKED) launch();

	player_condition = GREEN;
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
		// assume stars, planets too far away
		if (ships[i].shipType > BP_ESCAPEPOD) continue;

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

		if (ships[i].isHostile) gfx_SetColor(COLOR_YELLOW);
		else gfx_SetColor(COLOR_GREEN);
		gfx_Line(x, y, x, y + dy);
		gfx_SetPixel(x, y + dy);
		gfx_SetPixel(x - 1, y + dy);
	}

	// SOI indicator
	if (stationSoi) gfx_Sprite(stationsoi, SOI_INDIC_POS_X, SOI_INDIC_POS_Y);

	// compass
	for (unsigned char i = 0; i < numShips; i++)
	{
		// we are looking for the station if within its SOI (it has spawned in);
		// otherwise, we should just head towards the planet.
		if (ships[i].shipType != (stationSoi ? BP_CORIOLIS : PLANET)) continue;

		// draw the indicator on the compass
		const struct vector_t compassVector = normalize(ships[i].position);
		gfx_SetColor(compassVector.z > 0 ? COLOR_YELLOW : COLOR_GREEN);
		gfx_FillRectangle(COMPASS_HCENTER + compassVector.x / COMPASS_SCALE, 
						  COMPASS_VCENTER + compassVector.y / COMPASS_SCALE, 2, 1);
		break;
	}

	// right panel
	// speed display
	if (player_speed == PLAYER_MAX_SPEED) gfx_SetColor(COLOR_RED);
	else gfx_SetColor(COLOR_YELLOW);
	unsigned char const speedBarLength = 32 * player_speed / PLAYER_MAX_SPEED;
	gfx_FillRectangle(DASH_HOFFSET_RIGHT + 4, DASH_VOFFSET + 2, speedBarLength, 3);

	// roll display
	gfx_SetColor(COLOR_YELLOW);
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

void flt_TryHyperdrive()
{
	if (player_speed != PLAYER_MAX_SPEED) return;
	if (currentSeed.a == selectedSeed.a) return;
	if (currentSeed.b == selectedSeed.b) return;
	if (currentSeed.c == selectedSeed.c) return;

	stationSoi = false;
	gen_ChangeSystem();
}

void flt_TryInSystemJump()
{
	if (stationSoi) return; // definitely too close

	// check for enemies nearby
	for (unsigned char i = 0; i < numShips; i++)
	{
		if (ships[i].isHostile) return; // interference!
	}

	// find the planet
	for (unsigned char i = 0; i < numShips; i++)
	{
		if (ships[i].shipType != PLANET) continue;

		if (ships[i].position.z < 0x020000) return; // not facing planet or too close

		ships[i].position.z -= 0x010000; // do the jump
	}

	// we did jump, but now we need to move the sun, too
	// find the sun
	for (unsigned char i = 0; i < numShips; i++)
	{
		if (ships[i].shipType != SUN) continue;

		ships[i].position.z -= 0x010000; // move the sun
	}

	drawCycle = 0; // stimulate some enemies to spawn!
}

bool doFlightInput()
{
	updateKeys();

	// acceleration
	if (kb_IsDown(kb_Key2nd) && player_speed < PLAYER_MAX_SPEED) player_speed++;
	else if (kb_IsDown(kb_KeyAlpha) && player_speed > 0) player_speed--;
	
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

	if (clear && prevClear == 0) flt_TryHyperdrive();
	else if (vars && prevVars == 0) flt_TryInSystemJump();

	updatePrevKeys();

	if (graph && prevGraph == 1) // 1 bc we are after 1 update...
	{
		currentMenu = MAIN;
		return false;
	}
	else return true;
}

// this will only be called if we know that there is no station already
void flt_TrySpawnStation()
{
	// find the planet
	unsigned char planetIndex;
	for (planetIndex = 0; ships[planetIndex].shipType != PLANET; planetIndex++);

	// find the station's current (imaginary) position in its orbit
	const struct vector_t stationPos = 
		add(ships[planetIndex].position, mul(getCol(ships[planetIndex].orientation, 2), 2 * 96));

	// check distances
	if (intabs(stationPos.x) > 49152) return;
	if (intabs(stationPos.y) > 49152) return;
	if (intabs(stationPos.z) > 49152) return;

	// spawn the station
	struct Ship* station = NewShip(BP_CORIOLIS,
								   stationPos,
								   ships[planetIndex].orientation);

	// flip the nose vector so the slot faces the planet
	station->orientation.a[6] *= -1;
	station->orientation.a[7] *= -1;
	station->orientation.a[8] *= -1;

	// make the station spin
	station->roll = 127;

	stationSoi = true;
}

// these checks are pretty damn generous, but that's probably good for testing
unsigned char flt_CheckForDocking(unsigned char stationIndex)
{
	if (player_speed == 0)
		return 1;
	if (ships[stationIndex].position.z < 0) 
		return 1;

	bool successful = true;

	if (ships[stationIndex].orientation.a[8] < 115) 
		successful = false;
	if (ships[stationIndex].position.z < 119) 
		successful = false;
	if (ships[stationIndex].orientation.a[3] < 107 && ships[stationIndex].orientation.a[3] > -107) 
		successful = false;
	if (ships[stationIndex].isHostile)
		successful = false;

	if (successful) return 0;
	
	if (player_speed <= 5)
	{
		player_speed = 0;
		return 1;
	}
	
	flt_Death();
	return 2;
}

void flt_DoFrame(bool dashboardVisible)
{
	// black background
	gfx_FillScreen(COLOR_BLACK);

	// outer white frame
	gfx_SetColor(COLOR_WHITE);
	gfx_Rectangle(DASH_HOFFSET, 0, GFX_LCD_WIDTH - 2 * DASH_HOFFSET, GFX_LCD_HEIGHT - dashleft_height);

	// check if we have entered the station soi
	if (!stationSoi/* && drawCycle % 128 == 0*/) flt_TrySpawnStation();

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
	
		// let the ship move itself
		MoveShip(i);

		// check if we are close enough to dock/collide/grab
		if (ships[i].position.x > 191) continue;
		if (ships[i].position.x < -191) continue;
		if (ships[i].position.y > 191) continue;
		if (ships[i].position.y < -191) continue;
		if (ships[i].position.z > 191) continue;
		if (ships[i].position.z < -191) continue;

		// docking? break out of flight loop if we succeed
		if (ships[i].shipType == BP_CORIOLIS)
		{
			unsigned char dockOutcome = flt_CheckForDocking(i); // 0 = success
																// 1 = failure
																// 2 = death
			switch (dockOutcome)
			{
				case 0:

					player_condition = DOCKED;
					currentMenu = STATUS;
					numShips = 0;

					player_pitch = 0;
					player_roll = 0;
		
					flt_DoLaunchAnimation();

					return;

				case 2:

					player_dead = true;

					return;

				default: break; // if no dock, but no death, just keep going
			}
		}
	}

	drawSpaceView();

	if (dashboardVisible) drawDashboard();
}

void doFlight()
{
	while (doFlightInput())
	{
		clock_t frameTimer = clock();

		flt_DoFrame(true);
		if (player_condition == DOCKED) break;
		if (player_dead) break;
	
		while (clock() - frameTimer < FRAME_TIME);

		gfx_BlitBuffer();

		drawCycle++;
	}
}

void flt_Death()
{
	// we should have gotten here from the do damage routine,
	// but that could probably be called in lots of different
	// places... they all need to accomodate the possibility
	// of a game over

	drawCycle = 255;

	for (unsigned char i = 0; i < NUM_PLAYER_DEATH_CANS; i++)
	{
		struct Ship* can = NewShip(BP_CANISTER,
								   (struct vector_t){ 0, 0, 0 },
								   Matrix(256,0,0, 0,256,0, 0,0,256));
		can->speed = player_speed / 4;
		can->pitch = 127;
		can->roll = 127;
		can->isExploding = rand() % 2;
	}

	player_speed = 0;

	clock_t timer = clock();
	while (clock() - timer < DEATH_SCREEN_TIME)
	{
		clock_t frameTimer = clock();

		flt_DoFrame(false);
		xor_CenterText("GAME OVER", 9, VIEW_VCENTER);

		while (clock() - frameTimer < FRAME_TIME);

		gfx_BlitBuffer();
	}

	// TODO find a way to kick all the way back out of the game
}
