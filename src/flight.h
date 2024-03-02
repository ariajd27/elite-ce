#ifndef flight_include_file
#define flight_include_file

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

void flightInit();

void resetPlayerCondition();
void drawSpaceView();
void drawDashboard();
void doFlight();

#endif
