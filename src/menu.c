#include <stdlib.h>
#include "text.h"
#include "variables.h"
#include "flight.h"
#include "upgrades.h"
#include "menu.h"

void menu_DrawBackground()
{
	gfx_SetColor(COLOR_BLACK);
	gfx_FillRectangle(0, 0, GFX_LCD_WIDTH, GFX_LCD_HEIGHT);
	drawDashboard();
	gfx_SetColor(COLOR_WHITE);
	gfx_Rectangle(DASH_HOFFSET, 0, DASH_WIDTH, DASH_VOFFSET);
	// xor_HorizontalLine(HEADER_DIVIDER_Y, DASH_HOFFSET + 1, DASH_HOFFSET + DASH_WIDTH - 2);
	
	txt_SetCursorPos(0, 0);
}

void menu_PlayerStatus()
{
	currentMenu = STATUS;

	// name, position, navigation, condition label
	txt_SetCursorCol(7);
	txt_PutRecursive(126);

	// condition (TODO yellow and red)
	unsigned char statusToken = player_docked ? 129 : 70;
	txt_PutRecursive(statusToken);

	// fuel, money, legal status label
	txt_PutTokColon(125);

	// legal status
	statusToken = 133;
	if (player_outlaw > 0)
	{
		statusToken++;
		if (player_outlaw >= 50) statusToken++;
	}
	txt_PutRecursive(statusToken);

	// rating label
	txt_PutRecursive(130);

	// rating
	statusToken = 136;
	if (player_kills >= 4)
	{
		statusToken++;
		if (player_kills >= 8)
		{
			statusToken++;
			if (player_kills >= 16)
			{
				statusToken++;
				if (player_kills >= 32)
				{
					statusToken++;
					if (player_kills >= 256)
					{
						statusToken++;
						if (player_kills >= 512)
						{
							statusToken++;
							if (player_kills >= 2560)
							{
								statusToken++;
								if (player_kills >= 6400) statusToken++;
							}
						}
					}
				}
			}
		}
	}
	txt_PutRecursive(statusToken);

	// equipment label
	txt_PutTokCRLFTab(18);

	// equipment
	if (player_upgrades.largeCargoBay) txt_PutTokCRLFTab(107);
	if (player_upgrades.fuelScoops) txt_PutTokCRLFTab(111);
	if (player_upgrades.ecm) txt_PutTokCRLFTab(108);
	if (player_upgrades.energyBomb) txt_PutTokCRLFTab(113);
	if (player_upgrades.extraEnergy) txt_PutTokCRLFTab(114);
	if (player_upgrades.dockingComputer) txt_PutTokCRLFTab(115);
	if (player_upgrades.galacticHyperdrive) txt_PutTokCRLFTab(116);

	// weapons
	for (unsigned char i = 0; i < 4; i++)
	{
		txt_PutTokSpace(96 + i);
		txt_PutTokCRLFTab(103 + i);
	}
}

void menu_Market()
{
	// headers
	txt_SetCursorCol(17);
	txt_PutRecursive(95);
	txt_capsMode = TXT_TITLECASE;
	txt_lineSpacing = 1;
	
	// rows
	for (unsigned char item = 0; item < something; item++)
	{
		if (item == 3) continue; // slavery is bad

		// item name
		txt_SetCursorCol(1);
		txt_PutRecursive(48 + item);

		// item price
		txt_SetCursorCol(14);
		txt_PutUInt32(mkt_localPrices[item], 5, true);
	}
}
