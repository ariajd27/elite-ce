#ifndef upgrades_include_file
#define upgrades_include_file

typedef enum {
	PULSE,
	BEAM,
	MINING,
	MILITARY
} laser_t;

extern laser_t player_lasers[4];
extern unsigned char player_missiles;

extern struct player_upgrades_t {
	unsigned char largeCargoBay : 1;
	unsigned char ecm : 1;
	unsigned char fuelScoops : 1;
	unsigned char escapeCapsule : 1;
	unsigned char energyBomb : 1;
	unsigned char extraEnergy : 1;
	unsigned char dockingComputer : 1;
	unsigned char galacticHyperdrive : 1;
} player_upgrades;

void upg_PrintOutfittingTable();
bool upg_Buy(const unsigned char selIndex);

void upg_DisplayEquipment();

#endif
