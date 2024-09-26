#ifndef SHIP_H
#define SHIP_H

#include "linear.h"

#define MAX_NUM_SHIPS 10

typedef struct {

	unsigned char shipType;

	vector_t position;
	matrix_t orientation;

	unsigned char speed;
	signed char acceleration;
	signed char roll;
	signed char pitch;

	struct {
		unsigned char numMissiles : 4;
		unsigned char isExploding : 1;
		unsigned char isKilled : 1;
		unsigned char isOnScreen : 1;
		unsigned char isOnScanner : 1;
	} status;

	struct {
		unsigned char aggressiveness : 5;
		unsigned char hasAi : 1;
		unsigned char isHostile : 1;
		unsigned char hasEcm : 1;
	} ai;

	unsigned char *linesPtr;
	unsigned char energy;
	
	struct {
		unsigned char isTrader : 1;
		unsigned char isBountyHunter : 1;
		unsigned char isHostile : 1;
		unsigned char isPirate : 1;
		unsigned char isDocking : 1;
		unsigned char isInnocent : 1;
		unsigned char isCop : 1;
		unsigned char isDone : 1;
	} newb;

} ship_t;

extern ship_data_t sun;
extern ship_data_t planet;
extern ship_entry_t ships[MAX_NUM_SHIPS];
extern unsigned char numShips;
extern unsigned char numJunk;

ship_t shp_NewShip(const unsigned char shipType, const unsigned char shipIndex);
void shp_RemoveShip(const unsigned char shipIndex);

void shp_TidyOrientation(ship_t *ship);

#endif
