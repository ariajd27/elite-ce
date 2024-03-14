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
#include "upgrades.h"

#include <debug.h>

unsigned char player_speed;
signed char player_roll;
signed char player_pitch;

unsigned char player_altitude;
unsigned char player_cabin_temp;

unsigned char player_energy;
unsigned char player_fore_shield;
unsigned char player_aft_shield;

unsigned char player_laser_temp;
bool player_laser_overheat;
unsigned char laserPulseCounter;
bool drawLasers;

enum viewDirMode_t viewDirMode;
enum player_condition_t player_condition;

unsigned char junkAmt;
unsigned char extraSpawnDelay;

bool stationSoi;

void flt_Init()
{
	player_speed = 0;
	player_roll = 0;
	player_pitch = 0;

	player_altitude = 96;
	player_cabin_temp = 30;

	player_missiles = 3;

	player_energy = 255;
	player_fore_shield = 255;
	player_aft_shield = 255;

	player_laser_temp = 0;
	player_laser_overheat = false;
	laserPulseCounter = 0;

	viewDirMode = FRONT;
	player_condition = DOCKED;

	stationSoi = true;
}

void flt_DoTunnelAnimation(unsigned char step)
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

			xor_SteppedCircle(VIEW_HCENTER, VIEW_VCENTER, current, step);	
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

			xor_SteppedCircle(VIEW_HCENTER, VIEW_VCENTER, current, step);	
			gfx_BlitRectangle(gfx_buffer, xor_clipX, xor_clipY, xor_clipWidth, xor_clipHeight);

			while(clock() - timer < TUNNEL_FRAME_TIME);
		}
	}
}

void flt_DoLaunchAnimation()
{
	flt_DoTunnelAnimation(8);
}

void flt_DoHyperspaceAnimation()
{
	flt_DoTunnelAnimation(2);
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
	station->hasEcm = true;
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

	// energy banks
	unsigned char remainingEnergy = player_energy / 2;
	unsigned char barY = DASH_VOFFSET + 50;
	while (remainingEnergy > 0)
	{
		const unsigned char thisEnergy = remainingEnergy < 32 ? remainingEnergy : 32;

		gfx_SetColor(thisEnergy <= 4 ? COLOR_RED : COLOR_YELLOW);
		gfx_FillRectangle(DASH_HOFFSET_RIGHT + 4, barY, thisEnergy, 3);

		remainingEnergy -= thisEnergy;
		barY -= 8;
	}
	if (player_energy == 255) gfx_FillRectangle(DASH_HOFFSET_RIGHT + 35, DASH_VOFFSET + 26, 1, 3);

	// left panel
	// fore shield display
	gfx_SetColor(player_fore_shield < 64 ? COLOR_RED : COLOR_YELLOW);
	gfx_FillRectangle(DASH_HOFFSET + 20, DASH_VOFFSET + 2, player_fore_shield / 8 + 1, 3);

	// aft shield display
	gfx_SetColor(player_aft_shield < 64 ? COLOR_RED : COLOR_YELLOW);
	gfx_FillRectangle(DASH_HOFFSET + 20, DASH_VOFFSET + 10, player_aft_shield / 8 + 1, 3);

	// fuel display
	gfx_SetColor(COLOR_YELLOW);
	const unsigned char fuelBarLength = player_fuel <= 64 ? player_fuel / 2 : 32;
	gfx_FillRectangle(DASH_HOFFSET + 20, DASH_VOFFSET + 18, fuelBarLength, 3);

	// cabin temp display
	gfx_SetColor(player_cabin_temp > 192 ? COLOR_RED : COLOR_YELLOW);
	gfx_FillRectangle(DASH_HOFFSET + 20, DASH_VOFFSET + 26, player_cabin_temp / 8, 3);

	// laser temp display
	gfx_SetColor(player_laser_temp > 192 ? COLOR_RED : COLOR_YELLOW);
	gfx_FillRectangle(DASH_HOFFSET + 20, DASH_VOFFSET + 34, player_laser_temp / 8, 3);	

	// altitude display
	gfx_SetColor(player_altitude < 32 ? COLOR_RED : COLOR_YELLOW);
	gfx_FillRectangle(DASH_HOFFSET + 20, DASH_VOFFSET + 42, 1 + player_altitude / 8, 3);	
}

void flt_TryHyperdrive()
{
	if (player_speed != PLAYER_MAX_SPEED) return;
	if (currentSeed.a == selectedSeed.a) return;
	if (currentSeed.b == selectedSeed.b) return;
	if (currentSeed.c == selectedSeed.c) return;

	stationSoi = false;
	drawCycle = 0;
	junkAmt = 0;
	extraSpawnDelay = 0;
	gen_ChangeSystem();

	flt_DoHyperspaceAnimation();
}

void flt_TryInSystemJump()
{
	if (stationSoi) return; // definitely too close

	// check for other ships
	for (unsigned char i = 0; i < numShips; i++)
	{
		if (ships[i].shipType < BP_ASTEROID) return; // interference!
	}

	// find the sun and planet
	unsigned char sunIndex = 0;
	while (ships[sunIndex].shipType != SUN) sunIndex++;
	unsigned char planetIndex = 0;
	while (ships[planetIndex].shipType != PLANET) planetIndex++;

	// if both sun and planet are too close, the jump is impossible
	if (ships[sunIndex].position.z < 0x020000 && ships[planetIndex].position.z < 0x020000) return;
	
	// do the jump
	ships[planetIndex].position.z -= 0x010000;
	ships[sunIndex].position.z -= 0x010000;

	drawCycle = 0; // stimulate some enemies to spawn!
}

void flt_TryLasers()
{
	if (player_laser_overheat)
	{
		player_laser_overheat = player_laser_temp > 0;
		return;
	}

	if (player_laser_temp >= 242)
	{
		player_laser_overheat = true;
		return;
	}

	// there are only lasers on the front and back of the ship
	if (viewDirMode == LEFT || viewDirMode == RIGHT) return;

	// if we're shooting out the back, make sure the game realizes
	FlipAxes(viewDirMode);

	// if we have pulse lasers, make 'em pulse!
	if (laserPulseCounter > 0) return;
	if (player_lasers == PULSE) laserPulseCounter += PULSE_LASER_INTERVAL;

	// actually shoot: first step, pay to fire the lasers
	player_laser_temp += 8;
	player_energy -= 1;

	// laser lines can't be drawn here due to blit timings --- just set a flag to do it later
	drawLasers = true;

	// how much damage will we do if we hit?
	unsigned char const laserPower = 255; // TODO come up with a real number

	// find any ships hit and deal that much damage
	for (unsigned char i = 0; i < numShips; i++)
	{
		if (ships[i].position.z <= 0) continue; // not even in front of us
	
		// one does not simply shoot the sun
		if (ships[i].shipType == PLANET) continue;
		if (ships[i].shipType == SUN) continue;

		if (ships[i].isExploding) continue; // it's already dead, man!

		// is it even close enough to hit?
		if ((ships[i].position.x | ships[i].position.y) > LASER_MAX_RANGE) continue;

		// what is the ships's targetable area?
		const unsigned char* bpGeneral = bp_header_vectors[ships[i].shipType][BP_GENERAL];
		const unsigned int areaSquared = (bpGeneral[2] << 8) + bpGeneral[1];

		// now we can finally check the angle
		const unsigned int offsetSquared = ships[i].position.x * ships[i].position.x
										 + ships[i].position.y * ships[i].position.y;

		if (offsetSquared > areaSquared) continue; // we missed

		// if we made it down here, we hit ship i!!! yay!!!
		DamageShip(i, laserPower);
	}

	// put the axes back
	RestoreAxes(viewDirMode);
}

void flt_TryMissile()
{
	// TODO
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

	// engine controls
	if (clear && prevClear == 0) flt_TryHyperdrive();
	else if (vars && prevVars == 0) flt_TryInSystemJump();

	// weapons
	if (mode) flt_TryLasers();
	if (graphVar && prevGraphVar == 0) flt_TryMissile();

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

	// make the station spin
	station->roll = 127;

	stationSoi = true;
}

void flt_UpdatePlayerAltitude()
{
	for (unsigned char target = PLANET; true; target++)
	{
		if (target > SUN) 
		{
			// we have checked both sun and planet, and neither worked, so display a full bar
			player_altitude = 255;
			return;
		}

		// find what we are looking for
		unsigned char targetIndex = 0;
		bool found = true;
		while (ships[targetIndex].shipType != target)
		{
			targetIndex++;

			if (targetIndex >= numShips) // it doesn't exist
			{
				found = false;
				break;
			}
		}
		if (!found) continue; // we had to stop because we couldn't find the ship

		// check if it is close enough to display

		const signed int x = ships[targetIndex].position.x / 256;
		const signed int y = ships[targetIndex].position.y / 256;
		const signed int z = ships[targetIndex].position.z / 256;

		unsigned int altitude = intsqrt(x * x + y * y + z * z);

		if (altitude >= 255 + 96) continue; // too far

		player_altitude = altitude - 96;

		if (altitude <= 96) flt_Death(); // we have crashed into the planet!

		break; // if we made it down here, we drew the dial, so we are good
	}
}

void flt_UpdateCabinTemperature() // also handles fuel scooping
{
	// in case we kick out early, we need a default value
	player_cabin_temp = 30;

	if (stationSoi) return; // this prevents infinite searches for nonexistent suns,
							// and safely bc the station will always be far enough from
							// the sun to not need a temp check

	// find the sun
	unsigned char sunIndex = 0;
	while (ships[sunIndex].shipType != SUN) sunIndex++;


	unsigned int const sunX = intabs(ships[sunIndex].position.x) >> 8;
	unsigned int const sunY = intabs(ships[sunIndex].position.y) >> 8;
	unsigned int const sunZ = intabs(ships[sunIndex].position.z) >> 8;

	if (sunX > 256 || sunY > 256 || sunZ > 256) return; // if we are going to overflow on the multiplication,
														// we're too far away for this to matter anyway.

	unsigned int sunDistance = sunX * sunX + sunY * sunY + sunZ * sunZ;
	if (sunDistance > 256) return; // again, if it doesn't fit in a byte, too far, so kick out
	
	player_cabin_temp += sunDistance ^ 0xff;

	// if we overflowed, the cabin temp will have mysteriously reduced itself
	if (player_cabin_temp < 30) flt_Death();
}

// these checks are pretty damn generous, but that's probably good for testing
// returns 0 for success, 1 for failure, 2 for death
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

struct vector_t flt_GetSpawnPos()
{
	struct vector_t spawnPos = { rand() % 256, rand() % 256, 38 * 256 };
	if (rand() % 2) spawnPos.x += 512;	
	if (spawnPos.x & 0x80) spawnPos.x = -spawnPos.x;
	if (spawnPos.y & 0x80) spawnPos.y = -spawnPos.y;

	return spawnPos;
}

void flt_TrySpawnShips()
{
	if (stationSoi) return; // nothing spawns in the safe zone!
	if (numShips == MAX_SHIPS) return; // already full!

	if (rand() % 256 < 35)
	{
		// spawn some junk

		if (junkAmt >= 3) return; // too much junk already
	
		unsigned char junkType = rand() % 2 ? BP_COBRA
							   : rand() % 256 < 10 ? BP_CANISTER
							   : BP_ASTEROID;

		struct Ship* junk = NewShip(junkType, flt_GetSpawnPos(), Matrix(256,0,0, 0,256,0, 0,0,256));

		if (junkType == BP_COBRA) return; // will fly itself
		
		const signed char rollRand = (rand() % 128) | 0x6f;
		junk->roll = rand() % 2 ? rollRand : -rollRand;

		// we can't set both pitch and speed or else the junk will have a curved path (strange)
		if (rand() % 2) junk->speed = rand() % 16 + 16; // in range 16-31
		else junk->pitch = rand() % 2 ? 127 : -128; // max pitch, either up or down

		junkAmt++; // the cobra will fly away by itself, but we
				   // don't want canisters and rocks to build up
	}
	else
	{
		// check for cops scanning the player
		bool copNearby = false;
		for (unsigned char i = 0; i < numShips; i++) if (ships[i].shipType == BP_VIPER) copNearby = true;
		if (copNearby) mkt_GetScanned();

		// check for spawning a cop
		if (rand() % 256 < player_outlaw)
		{
			struct Ship* cop = NewShip(BP_VIPER, flt_GetSpawnPos(), Matrix(256,0,0, 0,256,0, 0,0,256));
			cop->aggro = 60;
			cop->isHostile = true;
			
			return;
		}

		// check if in grace period for enemies to spawn
		if (extraSpawnDelay > 0)
		{
			extraSpawnDelay--;
			return;
		}

		// check government type to spawn an enemy
		if ((rand() & 0x07) >= thisSystemData.government) return;

		// try spawning a bounty hunter
		if ((rand() % 256) < 100)
		{
			extraSpawnDelay++;
		
			const unsigned char hunterType = rand() % 2 ? BP_COBRA : BP_PYTHON;

			struct Ship* enemy = NewShip(hunterType, flt_GetSpawnPos(), Matrix(256,0,0, 0,256,0, 0,0,256));
			enemy->isHostile = true;
			enemy->aggro = 28;
			enemy->hasEcm = rand() % 256 >= 200; // 22% chance
			
			return;
		}

		// if no bounty hunter, spawn pirates
		for (unsigned char numPirates = rand() % 4 + 1; numPirates > 0; numPirates--)
		{
			extraSpawnDelay++;

			const unsigned char pirateType = rand() % 2 ? BP_MAMBA : BP_SIDEWINDER;

			struct Ship* enemy = NewShip(pirateType, flt_GetSpawnPos(), Matrix(256,0,0, 0,256,0, 0,0,256));
			enemy->isHostile = true;
			enemy->aggro = rand() % 64;
			enemy->hasEcm = rand() % 256 <= 10; // 4% chance

			if (numShips == MAX_SHIPS) break; // maxed out, can't spawn any more pirates
		}
	}
}

void flt_DoFrame(bool dashboardVisible)
{
	// black background
	gfx_FillScreen(COLOR_BLACK);

	// outer white frame
	gfx_SetColor(COLOR_WHITE);
	gfx_Rectangle(DASH_HOFFSET, 0, GFX_LCD_WIDTH - 2 * DASH_HOFFSET, GFX_LCD_HEIGHT - dashleft_height);

	// periodic updates
	if (drawCycle == 0) flt_TrySpawnShips(); // no need for "% 256" because drawCycle is a uint8
	if (!stationSoi && drawCycle % 64 == 0) flt_TrySpawnStation();
	if (drawCycle % 8 == 0) flt_UpdatePlayerAltitude();
	else if (drawCycle % 8 == 4) flt_UpdateCabinTemperature();

	// constant restoration
	if (laserPulseCounter > 0) laserPulseCounter--;
	if (player_laser_temp > 0) player_laser_temp--;
	if (player_energy < 255) player_energy++;

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

	// this flag is set when lasers are fired
	if (drawLasers)
	{
		const unsigned char laserY = VIEW_VCENTER - LASER_LINE_SPREAD / 2 + rand() % LASER_LINE_SPREAD;
		const unsigned char laserX = VIEW_HCENTER - LASER_LINE_SPREAD / 2 + rand() % LASER_LINE_SPREAD;

		xor_Line(laserX, laserY, LASER_LINE_X_ONE + DASH_HOFFSET, DASH_VOFFSET);
		xor_Line(laserX, laserY, LASER_LINE_X_TWO + DASH_HOFFSET, DASH_VOFFSET);
		xor_Line(laserX, laserY, DASH_WIDTH - LASER_LINE_X_ONE + DASH_HOFFSET, DASH_VOFFSET);
		xor_Line(laserX, laserY, DASH_WIDTH - LASER_LINE_X_TWO + DASH_HOFFSET, DASH_VOFFSET);

		drawLasers = false; // unset the flag for next frame
	}
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

	player_dead = true;

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
}
