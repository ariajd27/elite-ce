#include <stdlib.h>
#include "linear.h"
#include "ship.h"

ship_t sun;
ship_t planet;
ship_t ships[MAX_NUM_SHIPS];

unsigned char numShips = 0;
unsigned char numJunk = 0;

ship_t* shp_AddShip(const unsigned char shipType)
{
	if (numShips >= MAX_NUM_SHIPS) return 0;

	ship = &ships[numShips];
	ship->shipType = shipType;

	ship->orientation.vx = (vector_t){1, 0, 0};
	ship->orientation.vy = (vector_t){0, 1, 0};
	ship->orientation.vz = (vector_t){0, 0,-1};

	numShips++;
	return ship;
}

void shp_RemoveShip(const unsigned char shipIndex)
{
	free(ships[shipIndex].linesPtr);

	for (unsigned char i = shipIndex; i < MAX_NUM_SHIPS - 1; i++)
	{
		ships[i] = ships[i + 1];
		if (ships[i].shipType == 0) break;
	}
}

void shp_TidyOrientation(ship_t* ship)
{
	m_orthonormalize(&ship->orientation);
}
