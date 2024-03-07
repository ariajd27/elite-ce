#ifndef upgrades_include_file
#define upgrades_include_file

extern enum {
	PULSE,
	BEAM,
	MINING,
	MILITARY
} player_lasers;

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
void upg_Buy(const unsigned char selIndex);

#endif
