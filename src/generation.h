#ifndef generation_include_file
#define generation_include_file

#include <stdbool.h>

extern unsigned char gen_currentGalaxy;

struct gen_seed_t
{
	unsigned short a;
	unsigned short b;
	unsigned short c;
};

extern struct gen_seed_t originSeed, currentSeed, selectedSeed;

void gen_Twist(unsigned short* a, unsigned short* b, unsigned short* c);

struct gen_sysData_t
{
	char name[9]; // null-terminated

	unsigned char economy : 3;
	unsigned char government : 3;
	unsigned char techLevel : 4;
	unsigned char population : 7;
	unsigned int productivity;

	unsigned char x;
	unsigned char y;
};

extern struct gen_sysData_t thisSystemData, selectedSystemData;

extern signed int gen_crsX;
extern signed int gen_crsY;

extern unsigned int gen_distanceToTarget;
void gen_ResetDistanceToTarget();

void gen_SetSystemData(struct gen_sysData_t* out, struct gen_seed_t* in);

void gen_PrintName(const struct gen_seed_t* in, const bool lowercasify);
void gen_PrintEconomy(const struct gen_sysData_t* in);
void gen_PrintGovernment(const struct gen_sysData_t* in);
void gen_PrintTechnology(const struct gen_sysData_t* in);
void gen_PrintPopulation(const struct gen_sysData_t* data, const struct gen_seed_t* seed);
void gen_PrintProductivity(const struct gen_sysData_t* in);
void gen_PrintRadius(const struct gen_seed_t* in);

bool gen_PlanetHasCrater();

void gen_DrawLocalMap();
void gen_DrawGalaxyMap();
void gen_SelectNearestSystem(bool local);

void gen_ResetCursorPosition(bool local);
void gen_RedrawCursorPosition(signed int prevCrsX, signed int prevCrsY);

bool gen_ChangeSystem();

#endif
