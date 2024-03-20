#ifndef flight_include_file
#define flight_include_file

#include <stdbool.h>

enum viewDirMode_t {
	FRONT,
	LEFT,
	RIGHT,
	REAR
};

extern enum player_condition_t {
	DOCKED,
	GREEN,
	YELLOW,
	RED
} player_condition;

extern bool stationSoi;

extern unsigned char ecmTimer;

void flt_Init();

void resetPlayerCondition();
void drawSpaceView();
void drawDashboard();
void doFlight();

void flt_DoFrame(bool dashboardVisible);
void flt_Death();

#endif
