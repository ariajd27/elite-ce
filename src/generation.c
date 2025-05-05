#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <graphx.h>

#include "generation.h"
#include "xorgfx.h"
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
	struct gen_seed_t nameSeed = *in;

	unsigned char numIterations = (nameSeed.a & 0x0040) != 0 ? 4 : 3;
	unsigned char strIndex = 0;
	for (unsigned char i = 0; i < numIterations; i++)
	{
		unsigned char tknIndex = (nameSeed.c & 0x1f00) >> 7;
		gen_Twist(&nameSeed.a, &nameSeed.b, &nameSeed.c);
		if (tknIndex == 0) continue;

		if (token[tknIndex] != '?')
		{
			out->name[strIndex] = token[tknIndex];
			strIndex++;
		}
		if (token[tknIndex + 1] != '?')
		{
			out->name[strIndex] = token[tknIndex + 1];
			strIndex++;
		}
	}

	out->name[strIndex] = '\0';

	out->government = (in->b & 0x0038) >> 3;
	out->economy = (in->a & 0x0700) >> 8;
	if (out->government < 2) out->economy |= 2;

	out->techLevel = (~out->economy & 7) + ((in->b & 0x0300) >> 8) + (out->government + 1) / 2;
	out->population = out->techLevel * 4 + out->economy + out->government + 1;
	out->productivity = ((~out->economy & 7) + 3) * (out->government + 4) * out->population * 8;

	out->x = in->b >> 8;
	out->y = in->a >> 8;
}

void gen_PrintName(const struct gen_seed_t* in, const bool lowercasify)
{
	struct gen_seed_t nameSeed = *in;

	bool printLowercase = false;

	unsigned char numIterations = (nameSeed.a & 0x0040) != 0 ? 4 : 3;
	for (unsigned char i = 0; i < numIterations; i++)
	{
		unsigned char tknIndex = (nameSeed.c & 0x1f00) >> 7;
		gen_Twist(&nameSeed.a, &nameSeed.b, &nameSeed.c);
		if (tknIndex == 0) continue;

		if (token[tknIndex] != '?')
		{
			xor_PrintChar(token[tknIndex] + (printLowercase ? 32 : 0));
			printLowercase = lowercasify;
		}
		if (token[tknIndex + 1] != '?')
		{
			xor_PrintChar(token[tknIndex + 1] + (printLowercase ? 32 : 0));
			printLowercase = lowercasify;
		}
	}
}

void gen_PrintEconomy(const struct gen_sysData_t* in)
{
	switch (in->economy)
	{
		case 0:
		case 5:
			xor_Print("Rich ");
			break;
		case 1:
		case 6:
			xor_Print("Average ");
			break;
		case 2:
		case 7:
			xor_Print("Poor ");
			break;
		case 3:
		case 4:
			xor_Print("Mainly ");
	}

	if ((in->economy & 4) == 0) xor_Print("Industrial");
	else xor_Print("Agricultural");
}

void gen_PrintGovernment(const struct gen_sysData_t* in)
{
	switch(in->government)
	{
		case 0:
			xor_Print("Anarchy");
			break;
		case 1:
			xor_Print("Feudal");
			break;
		case 2:
			xor_Print("Multi-government");
			break;
		case 3:
			xor_Print("Dictatorship");
			break;
		case 4:
			xor_Print("Communist");
			break;
		case 5:
			xor_Print("Confederacy");
			break;
		case 6:
			xor_Print("Democracy");
			break;
		case 7:
			xor_Print("Corporate State");
	}
}

void gen_PrintTechnology(const struct gen_sysData_t* in)
{
	xor_PrintUInt8(in->techLevel + 1, 2);
}

void gen_PrintPopulation(const struct gen_sysData_t* data, const struct gen_seed_t* seed)
{
	xor_PrintChar('0' + data->population / 10);
	xor_PrintChar('.');
	xor_PrintChar('0' + data->population % 10);
	xor_Print(" Billion\n(");

	// get population species description
	if ((seed->c & 0x0080) == 0) xor_Print("Human Colonials");
	else
	{
		switch (seed->c & 0x1c00)
		{
			case 0x0000:
				xor_Print("Large ");
				break;
			case 0x0400:
				xor_Print("Fierce ");
				break;
			case 0x0800:
				xor_Print("Small ");
		}
		
		switch (seed->c & 0xe000)
		{
			case 0x0000:
				xor_Print("Green ");
				break;
			case 0x2000:
				xor_Print("Red ");
				break;
			case 0x4000:
				xor_Print("Yellow ");
				break;
			case 0x6000:
				xor_Print("Blue ");
				break;
			case 0x8000:
				xor_Print("Black ");
				break;
			case 0xa000:
				xor_Print("Harmless ");
		}

		unsigned int x = (seed->a ^ seed->b) & 0x0700;

		switch (x)
		{
			case 0x0000:
				xor_Print("Slimy ");
				break;
			case 0x0100:
				xor_Print("Bug-Eyed ");
				break;
			case 0x0200:
				xor_Print("Horned ");
				break;
			case 0x0300:
				xor_Print("Bony ");
				break;
			case 0x0400:
				xor_Print("Fat ");
				break;
			case 0x0500:
				xor_Print("Furry ");
		}

		switch ((x + (seed->c & 0x0300)) & 0x0700)
		{
			case 0x0000:
				xor_Print("Rodents");
				break;
			case 0x0100:
				xor_Print("Frogs");
				break;
			case 0x0200:
				xor_Print("Lizards");
				break;
			case 0x0300:
				xor_Print("Lobsters");
				break;
			case 0x0400:
				xor_Print("Birds");
				break;
			case 0x0500:
				xor_Print("Humanoids");
				break;
			case 0x0600:
				xor_Print("Felines");
				break;
			case 0x0700:
				xor_Print("Insects");
		}
	}

	xor_PrintChar(')');
}

void gen_PrintProductivity(const struct gen_sysData_t* in)
{
	xor_PrintUInt24(in->productivity, 5);
	xor_Print(" M CR");
}

void gen_PrintRadius(const struct gen_seed_t* in)
{
	xor_PrintUInt24(((in->c & 0x0f00) + 11 * 256) + (in->b >> 8), 4);
	xor_Print(" km");
}

void gen_PrintDistanceToTarget()
{
	xor_PrintUInt24Tenths(gen_distanceToTarget);
	xor_Print(" Light Years");
}

void gen_PrintSelectionOnMap()
{
	xor_SetCursorPos(0, xor_textRows - 1);
	gen_PrintName(&selectedSeed, false);
	xor_SetCursorPos(0, xor_textRows);
	xor_Print("Distance: ");
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

			xor_SetCursorPos((x - xor_clipX) / 8 + 1, row);
			gen_PrintName(&loopSeed, true);
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
	struct ship_t* planet = NewShip(PLANET,
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
