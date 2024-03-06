#ifndef ship_include_file
#define ship_include_file

#include <stdbool.h>

#include "linear.h"
#include "xorgfx.h"
#include "variables.h"
#include "flight.h"

#define SUN 101
#define PLANET 100

struct Ship
{
	unsigned char shipType;
	struct vector_t position;
	struct intmatrix_t orientation;

	bool isHostile;
	unsigned char aggro;

	unsigned char isExploding : 1;
	unsigned char toExplode : 1;
	unsigned char laserFiring : 1;

	unsigned char visibility : 5;
	unsigned char numVertices;
	unsigned char numEdges;
	unsigned char numFaces;

	unsigned char explosionSize;
	unsigned char explosionCount;
	unsigned char explosionRand[4];

	unsigned char speed;
	signed char acceleration;
	signed char pitch;
	signed char roll;
};

extern struct Ship ships[MAX_SHIPS];
extern unsigned char numShips;

struct Ship* NewShip(unsigned char shipType, struct vector_t position, struct intmatrix_t orientation);
void RemoveShip(unsigned char shipIndex);

struct int_point_t
{
	signed int x;
	signed int y;
};

struct int_point_t ProjPoint(struct vector_t toProject);
void ShipAsPoint(unsigned char shipIndex);
void ShipAsWireframe(unsigned char shipIndex);
void ShipAsPlanet(unsigned char shipIndex);
void DrawShip(unsigned char shipIndex);

void DoAI(unsigned char shipIndex);
void RotateShip(signed int* x, signed int* y, signed char amount);
void MoveShip(unsigned char shipIndex);
void FlipAxes(enum viewDirMode_t viewDirMode);
void RestoreAxes(enum viewDirMode_t viewDirMode);

#endif
