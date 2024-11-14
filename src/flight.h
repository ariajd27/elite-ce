#ifndef flight_include_file
#define flight_include_file

#include <stdbool.h>

typedef enum {
	DOCKED,
	GREEN,
	YELLOW,
	RED
} player_condition_t;

extern player_condition_t player_condition;

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

void flt_ResetPlayerCondition();
void drawSpaceView();
void drawDashboard();
void doFlight();

void flt_DoFrame(bool dashboardVisible);
void flt_DamagePlayer(unsigned char amount, bool fromBack);
void flt_Death();

#endif
