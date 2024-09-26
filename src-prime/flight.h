#ifndef flight_include_file
#define flight_include_file

#include <stdbool.h>

enum viewDirMode_t {
	FRONT,
	REAR,
	LEFT,
	RIGHT
};

extern bool playerDocked;
extern bool stationSoi;

extern unsigned char ecmTimer;

void flt_Init();

void resetPlayerCondition();
void drawSpaceView();
void drawDashboard();
void doFlight();

void flt_DoFrame(bool dashboardVisible);
void flt_DamagePlayer(unsigned char amount, bool fromBack);
void flt_Death();

#endif
