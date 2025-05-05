#ifndef ship_include_file
#define ship_include_file

#include <stdbool.h>

#include "linear.h"
#include "xorgfx.h"
#include "variables.h"
#include "flight.h"

// warning: there is code that relies on SUN > PLANET being true
#define SUN 101
#define PLANET 100

struct ship_t
{
	unsigned char shipType;
	struct vector_t position;
	struct intmatrix_t orientation;

	bool isHostile : 1;
	bool hasEcm : 1;
	unsigned char aggro;
	unsigned char target;

	unsigned char isExploding : 1;
	unsigned char toExplode : 1;
	unsigned char laserFiring : 1;

	unsigned char visibility : 5;
	unsigned char numVertices;
	unsigned char numEdges;
	unsigned char numFaces;

	unsigned char explosionSize;
	unsigned char explosionCount;
	unsigned char explosionRand;

	unsigned char speed;
	signed char acceleration;
	signed char pitch;
	signed char roll;
	
	unsigned char energy;
};

extern struct ship_t sun;
#define station sun
extern struct ship_t planet;
extern struct ship_t ships[MAX_SHIPS];
extern unsigned char numShips;

void InitShip(struct ship_t* ship, unsigned char shipType, struct vector_t position, struct intmatrix_t orientation);
struct ship_t* NewShip(unsigned char shipType, struct vector_t position, struct intmatrix_t orientation);
void RemoveShip(struct ship_t *ship);

struct int_point_t
{
	signed int x;
	signed int y;
};

struct int_point_t ProjPoint(struct vector_t toProject);
void ShipAsPoint(struct ship_t* ship);
void ShipAsWireframe(struct ship_t* ship);
void ShipAsBody(struct ship_t* ship);
void DrawShip(struct ship_t* ship);

void DoAI(unsigned char shipIndex);
void RotateShip(signed int* x, signed int* y, signed char amount);
void MoveShip(struct ship_t* ship);
void FlipAxes(enum viewDirMode_t viewDirMode);
void RestoreAxes(enum viewDirMode_t viewDirMode);

void DamageShip(unsigned char shipIndex, unsigned char damage);

#endif
