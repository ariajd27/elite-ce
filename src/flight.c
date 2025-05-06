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

player_condition_t player_condition = DOCKED;

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

bool playerDocked;
enum viewDirMode_t viewDirMode;

unsigned char junkAmt;
unsigned char extraSpawnDelay;
unsigned char ecmTimer;

enum {
	STANDBY,
	SEARCHING, 
	LOCKED
} missileStatus;
unsigned char missileTarget;

char flightMsg[FLTMSG_MAX_LENGTH];
unsigned char flightMsgLength;
unsigned char flightMsgTimer;

bool stationSoi;
bool flt_playerToDie;

void flt_Init()
{
	player_speed = 0;
	player_roll = 0;
	player_pitch = 0;

	player_altitude = 96;
	player_cabin_temp = 30;

	player_missiles = 3;
	missileStatus = STANDBY;

	player_energy = 255;
	player_fore_shield = 255;
	player_aft_shield = 255;

	player_laser_temp = 0;
	player_laser_overheat = false;
	laserPulseCounter = 0;

	viewDirMode = FRONT;
	flightMsgTimer = 0;
	stationSoi = true;
	flt_playerToDie = false;
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
	InitShip(&station, BP_CORIOLIS, (struct vector_t){ 0, 0, -256 }, Matrix(256,0,0, 0,256,0, 0,0,256));
	station.hasEcm = true;
	station.roll = 127;

	// spawn planet
	InitShip(&planet, PLANET, (struct vector_t){ 0, 0, 49152 }, Matrix(256,0,0, 0,256,0, 0,0,256));

	flt_DoLaunchAnimation();
}

void flt_SetMsg(char message[], unsigned char time)
{
	for (flightMsgLength = 0; message[flightMsgLength] != '\0'; flightMsgLength++)
	{
		flightMsg[flightMsgLength] = message[flightMsgLength];
	}

	flightMsgTimer = time;
}

void drawSpaceView()
{
	xor_SetCursorPos(11, 1);
	switch(viewDirMode)
	{
		case FRONT:
			xor_Print("Front");
			break;

		case REAR:
			xor_Print("Rear");
			break;

		case LEFT:
			xor_Print("Left");
			break;

		case RIGHT:
			xor_Print("Right");
			break;
	}
	xor_Print(" View");

	xor_Crosshair(VIEW_HCENTER, VIEW_VCENTER, CRS_SPREAD, CRS_SIZE);

	// ships
	FlipAxes(viewDirMode);
	DrawShip(&sun);
	DrawShip(&planet);
	for(unsigned char i = 0; i < numShips; i++) DrawShip(&ships[i]);
	RestoreAxes(viewDirMode);

	stardust_Move(viewDirMode, player_speed, player_pitch, player_roll);
	stardust_Draw(); // order is not really important here

	if (flightMsgTimer == 0) return; // no message to draw
	// TODO display flight message
	flightMsgTimer--;
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

		// don't draw dead ships
		if (ships[i].isExploding) continue;

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

	// missiles display
	for (unsigned char i = 0; i < player_missiles; i++)
	{
		gfx_SetColor(COLOR_GREEN);
		if (i + 1 == player_missiles)
		{
			switch (missileStatus)
			{
				case LOCKED:
					gfx_SetColor(COLOR_RED);
					break;
				case SEARCHING:
					gfx_SetColor(COLOR_YELLOW);
					break;
				default: // STANDBY
					break;
			}
		}

		gfx_FillRectangle(DASH_HOFFSET_CENTER - 10 - 8 * i, DASH_VOFFSET + 49, 6, 5);
	}
}

bool flt_CanJump()
{
	// check for other ships
	for (unsigned char i = 0; i < numShips; i++)
	{
		if (ships[i].shipType != BP_CORIOLIS && ships[i].shipType < BP_ASTEROID && !ships[i].isExploding)
		{
			flt_SetMsg("Interference!", FLTMSG_MED_TIME);
			return false;
		}
	}

	// find the sun and planet
	unsigned char planetIndex = 0;
	while (planet.shipType != PLANET) planetIndex++;

	if (!stationSoi)
	{
		unsigned char sunIndex = 0;
		while (sun.shipType != SUN) sunIndex++;
	
		// if both sun and planet are too close, the jump is impossible
		if (sun.position.z < 0x020000 && planet.position.z < 0x020000)
		{
			flt_SetMsg("Dangerous trajectory!", FLTMSG_MED_TIME);
			return false;
		}
	}
	else if (planet.position.z > 0
		  && intpow(planet.position.x >> 8, 2) 
		   + intpow(planet.position.y >> 8, 2) <= 9276)
	{
		flt_SetMsg("Dangerous trajectory!", FLTMSG_MED_TIME);
		return false;
	}

	if (player_speed != PLAYER_MAX_SPEED)
	{
		flt_SetMsg("Throttle up!", FLTMSG_MED_TIME);
		return false;
	}

	return true;
}

bool flt_CanHyperdrive()
{
	if (prgm && !player_upgrades.galacticHyperdrive) return false; // you gotta have the drive to use it!

	if (currentSeed.a == selectedSeed.a 
	 || currentSeed.b == selectedSeed.b 
	 || currentSeed.c == selectedSeed.c)
	{
		flt_SetMsg("No destination set!", FLTMSG_MED_TIME);
		return false;
	}

	return flt_CanJump();
}

void flt_DoHyperdrive(bool intergalactic)
{
	stationSoi = false;
	drawCycle = 0;
	junkAmt = 0;
	extraSpawnDelay = 0;

	if (intergalactic) gen_ChangeGalaxy();
	else gen_ChangeSystem();

	flt_DoHyperspaceAnimation();
}

void flt_TryInSystemJump()
{
	if (stationSoi) return; // definitely too close
	
	if (!flt_CanJump()) return;
	
	// find the sun and planet to do the jump
	unsigned char sunIndex = 0;
	while (sun.shipType != SUN) sunIndex++;
	unsigned char planetIndex = 0;
	while (planet.shipType != PLANET) planetIndex++;

	// do the jump
	planet.position.z -= 0x010000;
	sun.position.z -= 0x010000;

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
	if (player_lasers[viewDirMode] == PULSE) laserPulseCounter += PULSE_LASER_INTERVAL;

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
		// if ((ships[i].position.x | ships[i].position.y) > LASER_MAX_RANGE) continue;

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

void flt_TryFindMissileTarget()
{
	if (!player_missiles) return; // no missiles to shoot

	// who are we aiming at?
	for (missileTarget = 0; missileTarget < numShips; missileTarget++)
	{
		// is this a valid target?
		if (ships[missileTarget].position.z <= 0) continue;
		if (ships[missileTarget].shipType > PLANET) continue;
		if (ships[missileTarget].isExploding) continue;

		// is it close enough to see?
		const unsigned int z = intabs(ships[missileTarget].position.z);
		if (z > MAX_MISSILE_LOCK_RANGE) continue;

		// is it within a targeting arc?
		if (intabs(ships[missileTarget].position.x) > (z >> 3)) continue;
		if (intabs(ships[missileTarget].position.y) > (z >> 3)) continue;

		// we've found our target!
		missileStatus = LOCKED;
		break;
	}
}

void flt_LaunchMissile()
{	
	if (numShips + 1 >= MAX_SHIPS) return; // no room to spawn a missile

	struct ship_t* missile = NewShip(BP_MISSILE,
								   (struct vector_t){ 0, MISSILE_LAUNCH_Y, MISSILE_LAUNCH_Z },
								   Matrix(256,0,0, 0,256,0, 0,0,-256));
	missile->speed = MISSILE_LAUNCH_SPEED;
	missile->target = missileTarget;
	missile->aggro = 64;

	player_missiles--;

	// make the target mad on launch -- they see the missile lock
	if (ships[missileTarget].shipType != BP_ASTEROID) ships[missileTarget].isHostile = true;

	// reset for the next missile -- only if we actually managed to shoot
	missileStatus = STANDBY;
}

bool doFlightInput()
{
	updateKeys();

	// acceleration
	if (second > 0 && player_speed < PLAYER_MAX_SPEED) player_speed++;
	else if (alpha > 0 && player_speed > 0) player_speed--;
	
	if (yequ == 0) // pitch/roll controls
	{
		// roll
		if (left > 0) player_roll -= 6;
		else if (right > 0) player_roll += 6;

		// clamping
		if (player_roll > 32) player_roll = 32;
		else if (player_roll < -32) player_roll = -32;
	
		// pitch
		if (up > 0) player_pitch += 2;
		else if (down > 0) player_pitch -= 2;

		// clamping
		if (player_pitch > 8) player_pitch = 8;
		else if (player_pitch < -8) player_pitch = -8;
	}
	else // view switching
	{
		if (up > 0) viewDirMode = FRONT;
		else if (left > 0) viewDirMode = LEFT;
		else if (right > 0) viewDirMode = RIGHT;
		else if (down > 0) viewDirMode = REAR;
	}

	if (player_pitch < 0) player_pitch++;
	else if (player_pitch > 0) player_pitch--;
	if (player_roll < 0) player_roll += 2;
	else if (player_roll > 0) player_roll -= 2;	

	// engine controls
	if (((prgm | clear) > 0) && flt_CanHyperdrive())
	{
		if (clear >= STAR_JUMP_HOLD_TIME) flt_DoHyperdrive(false);
		else if (prgm >= GALAXY_JUMP_HOLD_TIME) flt_DoHyperdrive(true);
		else flt_SetMsg("Preparing jump...", 2);
	}
	else if (vars == 1) flt_TryInSystemJump();

	// weapons
	if (mode > 0) flt_TryLasers();
	if (graphVar == 1 && player_missiles > 0)
	{
		if (missileStatus == STANDBY) missileStatus = SEARCHING;
		else if (missileStatus == LOCKED) flt_LaunchMissile();
	}
	if (apps == 1) missileStatus = STANDBY;

	// only question left: do we exit or not?
	if (graph == 1)
	{
		currentMenu = MAIN;
		return false;
	}
	else return true;
}

// this will only be called if we know that there is no station already
void flt_TrySpawnStation()
{
	// find the station's current (imaginary) position in its orbit
	const struct vector_t stationPos = 
		add(planet.position, mul(getCol(planet.orientation, 2), 2 * 96));

	// check distances
	if (intabs(stationPos.x) > 49152) return;
	if (intabs(stationPos.y) > 49152) return;
	if (intabs(stationPos.z) > 49152) return;

	// spawn the station
	InitShip(&station, BP_CORIOLIS, stationPos, planet.orientation);

	// flip the nose vector so the slot faces the planet
	station.orientation.a[6] *= -1;
	station.orientation.a[7] *= -1;
	station.orientation.a[8] *= -1;

	// make the station spin
	station.roll = 127;

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
	while (sun.shipType != SUN) sunIndex++;


	unsigned int const sunX = intabs(sun.position.x) >> 8;
	unsigned int const sunY = intabs(sun.position.y) >> 8;
	unsigned int const sunZ = intabs(sun.position.z) >> 8;

	if (sunX > 256 || sunY > 256 || sunZ > 256) return; // if we are going to overflow on the multiplication,
														// we're too far away for this to matter anyway.

	unsigned int sunDistance = sunX * sunX + sunY * sunY + sunZ * sunZ;
	if (sunDistance > 256) return; // again, if it doesn't fit in a byte, too far, so kick out
	
	player_cabin_temp += sunDistance ^ 0xff;

	// if we overflowed, the cabin temp will have mysteriously reduced itself
	if (player_cabin_temp < 30) flt_Death();
}

// these checks are pretty damn generous, but that's probably good for testing
// returns 0 for success or 1 for failure
unsigned char flt_CheckForDocking()
{
	dbg_printf("considering checking for docking...\n");
	dbg_printf("station position: (%d, %d, %d)\n", station.position.x,
			station.position.y, station.position.z);
	dbg_printf("station orientation: [[%d, %d, %d], [%d, %d, %d], [%d, %d, %d]]\n", station.orientation.a[0],
			station.orientation.a[1], station.orientation.a[2], station.orientation.a[3],
			station.orientation.a[4], station.orientation.a[5], station.orientation.a[6],
			station.orientation.a[7], station.orientation.a[8]);

	if (player_speed == 0)
		return 1;
	if (station.position.z < 0) 
		return 1;

	bool successful = true;

	dbg_printf("checking station orientation...");
	if (station.orientation.a[8] > -115) 
	{
		dbg_printf(" failure!");
		successful = false;
	}

	dbg_printf("\nchecking station z position...");
	if (station.position.z < 119) 
	{
		dbg_printf(" failure!");
		successful = false;
	}

	dbg_printf("\nchecking station orientation...");
	if (station.orientation.a[3] < 107 && station.orientation.a[3] > -107) 
	{
		dbg_printf(" failure!");
		successful = false;
	}

	dbg_printf("\nchecking station hostility...");
	if (station.isHostile)
	{
		dbg_printf(" failure!");
		successful = false;
	}

	if (successful) return 0;

	dbg_printf("\ncheck unsucessful! ");
	
	if (player_speed <= 5)
	{
		dbg_printf("damaging player.\n");
		player_speed = 0;
		flt_DamagePlayer(15, FRONT);
	}
	else
	{
		dbg_printf("killing player.\n");
		flt_playerToDie = true;
	}

	return 1;
}

struct vector_t flt_GetSpawnPos()
{
	struct vector_t spawnPos = { rand() % 256, rand() % 256, 38 * 256 };
	if (rand() % 2) spawnPos.x += 512;	
	if (spawnPos.x & 0x80) spawnPos.x = -spawnPos.x;
	if (spawnPos.y & 0x80) spawnPos.y = -spawnPos.y;

	return spawnPos;
}

void flt_ResetPlayerCondition() {
	player_condition = GREEN;
	for (unsigned char i = 0; i < numShips; i++) {
		if (ships[i].shipType != BP_ASTEROID) player_condition = YELLOW;
	}
	if (player_condition == GREEN) return;
	if (player_energy < 255) player_condition = RED;
}

// this gets run every frame for every ship
// returns false usually, true if docking with station is successful
bool flt_UpdateShip(struct ship_t *ship)
{
	// apply speed to other ships
	ship->position.z -= player_speed;

	// apply pitch and roll to other ships' positions
	signed int oldY = ship->position.y - (player_roll * ship->position.x) / 256;
	ship->position.z += (player_pitch * oldY) / 256;
	ship->position.y = oldY - (player_pitch * ship->position.z) / 256;
	ship->position.x += (player_roll * ship->position.y) / 256;

	// apply pitch and roll to other ships' orientations
	for (unsigned char j = 0; j < 9; j += 3)
	{
		ship->orientation.a[j + 1] -= player_roll * ship->orientation.a[j + 0] / 256;
		ship->orientation.a[j + 0] += player_roll * ship->orientation.a[j + 1] / 256;
		ship->orientation.a[j + 1] -= player_pitch * ship->orientation.a[j + 2] / 256;
		ship->orientation.a[j + 2] += player_pitch * ship->orientation.a[j + 1] / 256;
	}

	// let the ship move itself
	MoveShip(ship);

	// no docking/colliding/grabbing when dead
	if (player_dead) return false;

	// check if we are close enough to dock/collide/grab
	if (ship->position.x > 191) return false;
	if (ship->position.x < -191) return false;
	if (ship->position.y > 191) return false;
	if (ship->position.y < -191) return false;
	if (ship->position.z > 191) return false;
	if (ship->position.z < -191) return false;

	// docking? break out of flight loop if we succeed
	if (ship != &station) return false;
	if (flt_CheckForDocking()) return false;

	dbg_printf("docking successful!\n");

	playerDocked = true;
	currentMenu = STATUS;
	numShips = 0;

	player_pitch = 0;
	player_roll = 0;

	flt_DoLaunchAnimation();

	return true;
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

		struct ship_t* junk = NewShip(junkType, flt_GetSpawnPos(), Matrix(256,0,0, 0,256,0, 0,0,256));

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
			struct ship_t* cop = NewShip(BP_VIPER, flt_GetSpawnPos(), Matrix(256,0,0, 0,256,0, 0,0,-256));
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

			struct ship_t* enemy = NewShip(hunterType, flt_GetSpawnPos(), Matrix(256,0,0, 0,256,0, 0,0,-256));
			enemy->isHostile = true;
			enemy->aggro = 28;
			enemy->hasEcm = rand() % 256 >= 200; // 22% chance
			enemy->speed = 12;

			return;
		}

		// if no bounty hunter, spawn pirates
		for (unsigned char numPirates = rand() % 4 + 1; numPirates > 0; numPirates--)
		{
			extraSpawnDelay++;

			const unsigned char pirateType = rand() % 2 ? BP_MAMBA : BP_SIDEWINDER;

			struct ship_t* enemy = NewShip(pirateType, flt_GetSpawnPos(), Matrix(256,0,0, 0,256,0, 0,0,-256));
			enemy->isHostile = true;
			enemy->aggro = rand() % 64;
			enemy->hasEcm = rand() % 256 <= 10; // 4% chance
			enemy->speed = 12;

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
	switch (drawCycle % 8)
	{
		case 0:
			flt_UpdatePlayerAltitude();
			break;

		case 4:
			flt_UpdateCabinTemperature();
			break;

		default: break;
	}

	// constant restoration
	if (laserPulseCounter > 0) laserPulseCounter--;
	if (player_laser_temp > 0) player_laser_temp--;
	if (player_energy < 255) player_energy++;
	if (ecmTimer > 0) ecmTimer--;

	// every cycle bc it's important for the physics system
	if (missileStatus == SEARCHING) flt_TryFindMissileTarget();

	// tidy vectors for each ship -- one ship per cycle
	ships[drawCycle % MAX_SHIPS].orientation = orthonormalize(ships[drawCycle % MAX_SHIPS].orientation);

	// do ai -- three ships per cycle
	DoAI(drawCycle % MAX_SHIPS);
	DoAI((drawCycle + MAX_SHIPS / 2) % MAX_SHIPS);

	flt_UpdateShip(&sun);
	if (flt_UpdateShip(&station)) return; // return if docking successful
	for (unsigned char i = 0; i < numShips; i++)
	{
		flt_UpdateShip(&ships[i]);
	}

	drawSpaceView();

	if (dashboardVisible) drawDashboard();

	if (!player_dead && flt_playerToDie)
	{
		flt_Death();
		return;
	}

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
	flt_playerToDie = false;

	while (doFlightInput())
	{
		clock_t frameTimer = clock();

		flt_DoFrame(true);
		if (player_dead) break;
	
		while (clock() - frameTimer < FRAME_TIME);

		gfx_BlitBuffer();

		drawCycle++;
	}
}

void flt_DamagePlayer(unsigned char amount, bool fromBack)
{
	unsigned char* relevantShield = fromBack ? &player_aft_shield : &player_fore_shield;
	
	// if the shield can sustain the damage, then it does, and we are done
	if (amount < *relevantShield)
	{
		*relevantShield -= amount;
		return;
	}

	// otherwise, we penetrate it and keep going
	amount -= *relevantShield;
	*relevantShield = 0;

	// if the energy banks can take it, then they do, and we are done
	if (amount < player_energy)
	{
		player_energy -= amount;
		return;
	}

	// otherwise, the player is dead :(
	flt_playerToDie = true;
}

void flt_Death()
{
	dbg_printf("killing player...\n");

	player_dead = true;

	drawCycle = 255;

	for (unsigned char i = 0; i < NUM_PLAYER_DEATH_CANS; i++)
	{
		struct ship_t* can = NewShip(BP_CANISTER,
								     (struct vector_t){ 0, 0, 0 },
								     Matrix(256,0,0, 0,256,0, 0,0,256));
		can->speed = player_speed / 4;
		can->pitch = 127;
		can->roll = 127;
		can->isExploding = rand() % 2;
	}

	player_speed = 0;

	dbg_printf("playing animation...\n");

	clock_t timer = clock();
	while (clock() - timer < DEATH_SCREEN_TIME)
	{
		clock_t frameTimer = clock();

		flt_DoFrame(false);
		xor_SetCursorPos(9, 9);
		xor_Print("GAME OVER");

		while (clock() - frameTimer < FRAME_TIME);

		gfx_BlitBuffer();
	}

	dbg_printf("done with death animation!\n");
}
