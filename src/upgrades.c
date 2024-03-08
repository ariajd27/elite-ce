#include <stdbool.h>

#include "generation.h"
#include "variables.h"
#include "xorgfx.h"

enum {
	PULSE,
	BEAM,
	MINING,
	MILITARY
} player_lasers;

unsigned char player_missiles;

struct {
	unsigned char largeCargoBay : 1;
	unsigned char ecm : 1;
	unsigned char fuelScoops : 1;
	unsigned char escapeCapsule : 1;
	unsigned char energyBomb : 1;
	unsigned char extraEnergy : 1;
	unsigned char dockingComputer : 1;
	unsigned char galacticHyperdrive : 1;
} player_upgrades;

const char upg_displayNames[NUM_UPGRADES][20] = {
	"Missile",
	"Large Cargo Bay",
	"ECM System",
	"Pulse Lasers",
	"Beam Lasers",
	"Fuel Scoops",
	"Escape Capsule",
	"Energy Bomb",
	"Extra Energy Unit",
	"Docking Computers",
	"Galactic Hyperdrive",
	"Mining Lasers",
	"Military Lasers"
};

const unsigned int upg_prices[NUM_UPGRADES + 1] = {
	300,
	4000,
	6000,
	4000,
	10000,
	5250,
	10000,
	9000,
	15000,
	15000,
	50000,
	8000,
	60000
};

void upg_PrintOutfittingTable()
{
	xor_SetCursorPos(1, 3);
	xor_Print("ITEM");
	xor_SetCursorPos(23, 3);
	xor_Print("PRICE");

	xor_SetCursorPos(0, 4);
	xor_Print("Fuel");
	xor_SetCursorPos(22, 4);
	xor_PrintUInt8((70 - player_fuel) * 2, 3);
	xor_PrintChar('.');
	xor_PrintUInt8((70 - player_fuel) * 20 % 10, 1);
	xor_Print(" Cr");

	for (unsigned char i = 0; i < NUM_UPGRADES; i++)
	{
		// reqTL is actually just i clamped to [0, 10]
		const unsigned char reqTL = i == 0 ? 0 
								  : i < 11 ? i - 1 
								  : 10;

		if (reqTL > thisSystemData.techLevel) break;

		const unsigned char y = i + 5;

		xor_SetCursorPos(0, y);
		xor_Print(upg_displayNames[i]);

		xor_SetCursorPos(21, y);
		xor_PrintUInt24(upg_prices[i] / 10, 4);
		xor_PrintChar('.');
		xor_PrintUInt8(upg_prices[i] % 10, 1);
		xor_Print(" Cr");
	}

	xor_SetCursorPos(0, NUM_UPGRADES + 8);
	xor_Print("Cash: ");
	xor_PrintUInt24Tenths(player_money);
	xor_Print(" Cr");
}

bool upg_Buy(const unsigned char selIndex)
{
	const unsigned int price = selIndex == 0 ? (70 - player_fuel) * 20 : upg_prices[selIndex - 1];
	if (price > player_money) return false;

	// we check for already having the thing on a case by case basis
	switch (selIndex)
	{
		case 0:
			if (player_fuel >= 70) return false;
			player_fuel = 70;
			break;

		case 1:
			if (player_missiles >= 4) return false;
			player_missiles++;
			break;

		case 2:
			if (player_upgrades.largeCargoBay) return false;
			player_cargo_space += 10;
			player_upgrades.largeCargoBay = true;
			break;

		case 3:
			if (player_upgrades.ecm) return false;
			player_upgrades.ecm = true;
			break;

		case 4:
			if (player_lasers == PULSE) return false;
			player_lasers = PULSE;
			break;

		case 5:
			if (player_lasers == BEAM) return false;
			player_lasers = BEAM;
			break;

		case 6:
			if (player_upgrades.fuelScoops) return false;
			player_upgrades.fuelScoops = true;
			break;

		case 7:
			if (player_upgrades.escapeCapsule) return false;
			player_upgrades.escapeCapsule = true;
			break;

		case 8:
			if (player_upgrades.energyBomb) return false;
			player_upgrades.energyBomb = true;
			break;

		case 9:
			if (player_upgrades.extraEnergy) return false;
			player_upgrades.extraEnergy = true;
			break;

		case 10:
			if (player_upgrades.dockingComputer) return false;
			player_upgrades.dockingComputer = true;
			break;

		case 11:
			if (player_upgrades.galacticHyperdrive) return false;
			player_upgrades.galacticHyperdrive = true;
			break;

		case 12:
			if (player_lasers == MINING) return false;
			player_lasers = MINING;
			break;

		case 13:
			if (player_lasers == MILITARY) return false;
			player_lasers = MILITARY;
			break;

		default:
			return false; // we have somehow selected a nonexistent item. don't buy it.
	}

	// if we get here, we got the upgrade, so we should probably pay for it
	player_money -= price;

	return true;
}

void upg_DisplayEquipment()
{
	const unsigned char x = xor_cursorX;
	unsigned char y = xor_cursorY + 1; // incremented after each line printed,
									   // already at +1 bc lasers always printed

	// always print equipped lasers
	xor_Print(upg_displayNames[player_lasers == PULSE ? 3
							 : player_lasers == BEAM ? 4
							 : player_lasers == MINING ? 11
							 : 12]);

	if (player_upgrades.largeCargoBay)
	{
		xor_SetCursorPos(x, y);
		xor_Print(upg_displayNames[1]);
		y++;
	}
	if (player_upgrades.ecm)
	{
		xor_SetCursorPos(x, y);
		xor_Print(upg_displayNames[2]);
		y++;
	}
}
