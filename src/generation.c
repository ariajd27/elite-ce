#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <graphx.h>

#include "generation.h"
#include "xorgfx.h"
#include "text.h"
#include "variables.h"
#include "market.h"
#include "ship.h"

#include <debug.h>

unsigned char gen_currentGalaxy;

struct gen_seed_t originSeed, currentSeed, selectedSeed;
struct gen_sysData_t thisSystemData, selectedSystemData;

signed int gen_crsX;
signed int gen_crsY;

unsigned int gen_distanceToTarget;

const char token[64] = {
	'A', 'L', 'L', 'E', 'X', 'E', 'G', 'E', 
	'Z', 'A', 'C', 'E', 'B', 'I', 'S', 'O',
	'U', 'S', 'E', 'S', 'A', 'R', 'M', 'A',
	'I', 'N', 'D', 'I', 'R', 'E', 'A', '?',
	'E', 'R', 'A', 'T', 'E', 'N', 'B', 'E',
	'R', 'A', 'L', 'A', 'V', 'E', 'T', 'I',
	'E', 'D', 'O', 'R', 'Q', 'U', 'A', 'N',
	'T', 'E', 'I', 'S', 'R', 'I', 'O', 'N'
};

void gen_Twist(unsigned short* a, unsigned short* b, unsigned short* c)
{
	unsigned int newC = *a + *b + *c;
	*a = *b;
	*b = *c;
	*c = newC;
}

void gen_ResetDistanceToTarget()
{
	unsigned int const dX = 4 * abs((selectedSeed.b >> 8) - (currentSeed.b >> 8));
	unsigned int const dY = 2 * abs((selectedSeed.a >> 8) - (currentSeed.a >> 8));
	gen_distanceToTarget = sqrt(dX * dX + dY * dY);
}

void gen_SetSystemData(struct gen_sysData_t* out, struct gen_seed_t* in)
{
	out->government = (in->b & 0x0038) >> 3;
	out->economy = (in->a & 0x0700) >> 8;
	if (out->government < 2) out->economy |= 2;

	out->techLevel = (~out->economy & 7) + ((in->b & 0x0300) >> 8) + (out->government + 1) / 2;
	out->population = out->techLevel * 4 + out->economy + out->government + 1;
	out->productivity = ((~out->economy & 7) + 3) * (out->government + 4) * out->population * 8;

	out->x = in->b >> 8;
	out->y = in->a >> 8;
}

void gen_PrintName(const struct gen_seed_t* in)
{
	struct gen_seed_t nameSeed = *in;

	unsigned char numIterations = (nameSeed.a & 0x0040) != 0 ? 4 : 3;
	for (unsigned char i = 0; i < numIterations; i++)
	{
		unsigned char tknIndex = (nameSeed.c & 0x1f00) >> 7;
		gen_Twist(&nameSeed.a, &nameSeed.b, &nameSeed.c);
		if (tknIndex == 0) continue;

		txt_PutTok(tknIndex & 0x80);
	}
}

void gen_PrintEconomy(const struct gen_sysData_t* in)
{
	switch (in->economy)
	{
		case 0:
		case 5:
			txt_PutRecursive(10);
			break;
		case 1:
		case 6:
			txt_PutRecursive(11);
			break;
		case 2:
		case 7:
			txt_PutRecursive(12);
			break;
		case 3:
		case 4:
			txt_PutRecursive(13);
			break;
	}

	if ((in->economy & 4) == 0) txt_PutRecursive(8);
	else txt_PutRecursive(9);
}

void gen_PrintGovernment(const struct gen_sysData_t* in)
{
	txt_PutRecursive(in->government + 17);
}

void gen_PrintTechnology(const struct gen_sysData_t* in)
{
	txt_PutUInt32(in->techLevel + 1, 2, false);
}

void gen_PrintPopulation(const struct gen_sysData_t* data, const struct gen_seed_t* seed)
{
	txt_PutUInt32(data->population, 2, true);
	txt_PutRecursive(38);
	txt_CRLF();

	// get population species description
	txt_PutC('(' - 32);
	if ((seed->c & 0x0080) == 0) txt_PutRecursive(28);
	else
	{
		// size
		unsigned int x = seed->c & 0x1c00;
		if (x <= 0x0800) txt_PutRecursive((x >> 10) + 67);

		// color
		x = seed->c & 0xe000;
		if (x <= 0xa000) txt_PutRecursive((x >> 13) + 70);

		// texture
		x = (seed->a ^ seed->b) & 0x0700;
		if (x <= 0x0500) txt_PutRecursive((x >> 8) + 76);

		// type
		txt_PutRecursive(((x + (seed->c & 0x0300)) & 0x0700) + 82);
		txt_PutC('S' - 32);
	}
	txt_PutC(')' - 32);
}

void gen_PrintProductivity(const struct gen_sysData_t* in)
{
	txt_PutUInt32(in->productivity, 5, false);
	txt_PutString(" M");
	txt_PutRecursive(66);
}

void gen_PrintRadius(const struct gen_seed_t* in)
{
	txt_PutUInt32(((in->c & 0x0f00) + 11 * 256) + (in->b >> 8), 4, false);
	txt_PutString(" km");
}

void gen_PrintDistanceToTarget()
{
	txt_PutUInt32(gen_distanceToTarget, 10, true);
	txt_PutRecursive(35);
}

void gen_PrintSelectionOnMap()
{
	txt_SetCursorPos(0, xor_textRows - 1);
	gen_PrintName(&selectedSeed);
	txt_SetCursorPos(0, xor_textRows);
	txt_PutTokColon(191);
	gen_PrintDistanceToTarget();
}

void gen_DrawLocalMap()
{
	// fixed elements
	xor_Crosshair(LCL_MAP_HFIX, LCL_MAP_VFIX, CRS_SPREAD, CRS_SIZE);	
	xor_Circle(LCL_MAP_HFIX, LCL_MAP_VFIX, player_fuel); // current range illustration

	if (gen_crsX < 4 * LCL_MAP_DXMAX && gen_crsX > -4 * LCL_MAP_DXMAX
			&& gen_crsY < 2 * LCL_MAP_DYMAX && gen_crsY > -2 * LCL_MAP_DYMAX)
	{
		xor_Crosshair(LCL_MAP_HFIX + gen_crsX, LCL_MAP_VFIX + gen_crsY, SML_CRS_SPREAD, SML_CRS_SIZE);
	}

	struct gen_seed_t loopSeed = originSeed;
	bool labelOnLine[xor_textCols] = { 0 };
	
	unsigned char i = 0;
	do
	{
		if (i != 0) 
		{
			gen_Twist(&loopSeed.a, &loopSeed.b, &loopSeed.c);
			gen_Twist(&loopSeed.a, &loopSeed.b, &loopSeed.c);
			gen_Twist(&loopSeed.a, &loopSeed.b, &loopSeed.c);
			gen_Twist(&loopSeed.a, &loopSeed.b, &loopSeed.c);
		}

		i++;

		signed int const dx = (signed int)(loopSeed.b >> 8) - (signed int)thisSystemData.x;
		if (dx >= LCL_MAP_DXMAX || dx <= -1 * LCL_MAP_DXMAX) continue;

		signed int const dy = (signed int)(loopSeed.a >> 8) - (signed int)thisSystemData.y;
		if (dy >= LCL_MAP_DYMAX || dy <= -1 * LCL_MAP_DYMAX) continue;

		unsigned int const x = LCL_MAP_HFIX + 4 * dx;
		unsigned char const y = LCL_MAP_VFIX + 2 * dy;

		// this deviates from the original game because in order to get the same
		// display sizes for each system, we would have to implement the same
		// text drawing subroutines as the original. i am not doing that. myeh.
		xor_FillCircle(x, y, ((loopSeed.c & 0x0100) >> 8) + 3);
		
		unsigned char row = y / 8;
		if (labelOnLine[row]) row += 1;
		if (labelOnLine[row]) row -= 2;
		if (!labelOnLine[row] && row > 2)
		{
			labelOnLine[row] = true;

			txt_SetCursorPos((x - xor_clipX) / 8 + 1, row);
			gen_PrintName(&loopSeed);
		}
	}
	while (i != 0);

	gen_PrintSelectionOnMap();
}

void gen_DrawGalaxyMap()
{
	xor_Circle(GLX_MAP_HOFFSET + thisSystemData.x, GLX_MAP_VOFFSET + thisSystemData.y / 2, player_fuel / 4);
	xor_Crosshair(GLX_MAP_HOFFSET + gen_crsX, GLX_MAP_VOFFSET + gen_crsY, SML_CRS_SPREAD, SML_CRS_SIZE);

	struct gen_seed_t loopSeed = originSeed;
	unsigned char i = 0;
	do
	{
		if (i != 0)
		{
			gen_Twist(&loopSeed.a, &loopSeed.b, &loopSeed.c);
			gen_Twist(&loopSeed.a, &loopSeed.b, &loopSeed.c);
			gen_Twist(&loopSeed.a, &loopSeed.b, &loopSeed.c);
			gen_Twist(&loopSeed.a, &loopSeed.b, &loopSeed.c);
		}

		i++;

		unsigned char const x = loopSeed.b >> 8;
		unsigned char const y = loopSeed.a >> 9;

		xor_Point(GLX_MAP_HOFFSET + x, GLX_MAP_VOFFSET + y);
	}
	while (i != 0);

	gen_PrintSelectionOnMap();
}

void gen_ResetCursorPosition(bool local)
{
	if (local)
	{
		gen_crsX = 4 * (selectedSystemData.x - thisSystemData.x);
		gen_crsY = 2 * (selectedSystemData.y - thisSystemData.y);
	}
	else
	{
		gen_crsX = selectedSystemData.x;
		gen_crsY = selectedSystemData.y / 2;
	}
}

void gen_RedrawCursorPosition(signed int const prevCrsX, signed int const prevCrsY)
{
	if (currentMenu == LOCAL_MAP)
	{
		if (gen_crsX < 4 * LCL_MAP_DXMAX && gen_crsX > -4 * LCL_MAP_DXMAX
				&& gen_crsY < 2 * LCL_MAP_DYMAX && gen_crsY > -2 * LCL_MAP_DYMAX)
		{
			xor_Crosshair(LCL_MAP_HFIX + gen_crsX, LCL_MAP_VFIX + gen_crsY, SML_CRS_SPREAD, SML_CRS_SIZE);
		}
		if (prevCrsX < 4 * LCL_MAP_DXMAX && prevCrsX > -4 * LCL_MAP_DXMAX
				&& prevCrsY < 2 * LCL_MAP_DYMAX && prevCrsY > -2 * LCL_MAP_DYMAX)
		{
			xor_Crosshair(LCL_MAP_HFIX + prevCrsX, LCL_MAP_VFIX + prevCrsY, SML_CRS_SPREAD, SML_CRS_SIZE);
		}
	}
	else
	{
		xor_Crosshair(GLX_MAP_HOFFSET + prevCrsX, GLX_MAP_VOFFSET + prevCrsY, SML_CRS_SPREAD, SML_CRS_SIZE);
		xor_Crosshair(GLX_MAP_HOFFSET + gen_crsX, GLX_MAP_VOFFSET + gen_crsY, SML_CRS_SPREAD, SML_CRS_SIZE);
	}

	gfx_BlitRectangle(gfx_buffer, xor_clipX, xor_clipY, xor_clipWidth, xor_clipHeight);
}

void gen_SelectNearestSystem(bool local)
{
	gen_PrintSelectionOnMap();

	struct gen_seed_t loopSeed = originSeed;

	unsigned char crsXDescaled = local ? thisSystemData.x + gen_crsX / 4 : gen_crsX;
	unsigned char crsYDescaled = local ? thisSystemData.y + gen_crsY / 2 : gen_crsY * 2;

	struct gen_seed_t bestSeed;
	unsigned int bestR = 32768; // 4 * 128 * 128 + 128 * 128

	unsigned char i = 0;
	do
	{
		if (i != 0)
		{
			gen_Twist(&loopSeed.a, &loopSeed.b, &loopSeed.c);
			gen_Twist(&loopSeed.a, &loopSeed.b, &loopSeed.c);
			gen_Twist(&loopSeed.a, &loopSeed.b, &loopSeed.c);
			gen_Twist(&loopSeed.a, &loopSeed.b, &loopSeed.c);
		}

		i++;

		signed int const dX = (unsigned char)(loopSeed.b >> 8) - crsXDescaled;
		signed int const dY = (unsigned char)(loopSeed.a >> 8) - crsYDescaled;
		unsigned int const r = dX * dX + dY * dY;
		if (r >= bestR) continue;

		bestSeed = loopSeed;
		bestR = r;

	}
	while (i != 0);

	selectedSeed = bestSeed;
	gen_SetSystemData(&selectedSystemData, &selectedSeed);
	gen_ResetDistanceToTarget();	
	gen_ResetCursorPosition(local);
	gen_PrintSelectionOnMap();

	gfx_BlitRectangle(gfx_buffer, xor_clipX, xor_clipY, xor_clipWidth, xor_clipHeight);
}

bool gen_ChangeSystem()
{
	if (player_fuel < gen_distanceToTarget) return false; // this is the only failure condition

	player_fuel -= gen_distanceToTarget;
	currentSeed = selectedSeed;
	thisSystemData = selectedSystemData;
	gen_distanceToTarget = 0;

	marketSeed = rand() % 256;
	mkt_ResetLocalMarket();

	numShips = 0;

	// originally this rand() call should actually be determined by the carry flag
	// after reducing the player's legal status, which means that if you never commit
	// any crimes, planets will always appear in the same place on your screen...
	// i think. i'm not sure. well, since I don't know how to access the carry flag
	// anyway, i've just randomized it.
	signed int planetZ = (((currentSeed.a & 0x0700) >> 8) + 6) << 15;
	signed int planetX = planetZ >> 1;
	if (rand() % 2 == 0) planetX *= -1;
	struct Ship* planet = NewShip(PLANET,
								  (struct vector_t){ planetX, planetX, planetZ },
								  Matrix(256,0,0, 0,256,0, 0,0,256));
	planet->pitch = 127;
	planet->roll = 127;

	signed int sunZ = -(((currentSeed.b & 0x0600) | 0x0100) << 8);
	signed int sunX = (currentSeed.c & 0x0300) << 8;
	NewShip(SUN,
		    (struct vector_t){ sunX, sunX, sunZ },
		    Matrix(256,0,0, 0,256,0, 0,0,256));

	return true;
}

bool gen_PlanetHasCrater()
{
	return (thisSystemData.techLevel & 0x02) != 0;
}

void gen_RotateBytes(unsigned char *start)
{
	unsigned char savedBit = (*start & 0x80) >> 7;
	*start = (*start << 1) | savedBit;
	savedBit = (*(start + 1) & 0x80) >> 7;
	*(start + 1) = (*(start + 1) << 1) | savedBit;
}

void gen_ChangeGalaxy() // also changes systems!
{
	gen_RotateBytes((unsigned char*)&originSeed.a);
	gen_RotateBytes((unsigned char*)&originSeed.b);
	gen_RotateBytes((unsigned char*)&originSeed.c);
	gen_currentGalaxy++;

	selectedSeed = originSeed;
	gen_SetSystemData(&selectedSystemData, &selectedSeed);
	gen_distanceToTarget = 70; // an intergalactic jump takes a full fuel tank
	gen_ChangeSystem();
}
