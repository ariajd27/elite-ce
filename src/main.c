#include <graphx.h>
#include <keypadc.h>
#include <sys/rtc.h>

#include "gfx/gfx.h"

#include "xorgfx.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "variables.h"

#include "ship.h"
#include "ship_data.h"
#include "stardust.h"
#include "generation.h"
#include "market.h"
#include "flight.h"
#include "input.h"

#include <debug.h>

bool toExit = false;
unsigned char drawCycle;

enum currentMenu_t currentMenu;
unsigned char menu_selOption = 0;

bool player_dead;

unsigned char player_fuel;
unsigned int player_money;
unsigned char player_outlaw;
unsigned int player_kills;

unsigned char player_cargo_space;
unsigned char player_cargo_cap;

void printPlayerCondition()
{
	switch (player_condition)
	{
		case DOCKED:
			xor_Print("Docked");
			break;
		case GREEN:
			xor_Print("Green");
			break;
		case YELLOW:
			xor_Print("Yellow");
			break;
		case RED:
			xor_Print("Red");
			break;
		default:
			xor_Print("Unknown");
	}
}

void drawMenu(bool resetCrs)
{
	gfx_SetColor(COLOR_BLACK);
	gfx_FillRectangle(0, 0, GFX_LCD_WIDTH, GFX_LCD_HEIGHT);
	drawDashboard();
	gfx_SetColor(COLOR_WHITE);
	gfx_Rectangle(DASH_HOFFSET, 0, DASH_WIDTH, DASH_VOFFSET);
	xor_HorizontalLine(HEADER_DIVIDER_Y, DASH_HOFFSET + 1, DASH_HOFFSET + DASH_WIDTH - 2);

	if (resetCrs) menu_selOption = 0;

	switch (currentMenu)
	{
		case MAIN:

			xor_CenterText("SHIP COMPUTER", 13, HEADER_Y); 

			xor_SetCursorPos(0, 4);
			xor_lineSpacing = true;

			xor_Print("Status\n");
			xor_Print("Navigation\n");
			xor_Print("System Data\n");

			if (player_condition == DOCKED)
			{
				xor_Print("Sell Cargo\n");
				xor_Print("Leave Station\n");
				xor_Print("Save & Quit");
			}
			else
			{
				xor_Print("Cargo Hold\n");
				xor_Print("Return\n");
				xor_Print("Quit");
			}

			xor_FillRectangle(xor_clipX + LEFT_TEXT_INDENT - 2,
					29 + 16 * menu_selOption, MM_SELBAR_WIDTH, 11);

			break;
			
		case STATUS:

			xor_CenterTextOffset("COMMANDER", 9, HEADER_Y, 1 + cmdr_name_length);
			xor_CenterTextOffset(cmdr_name, cmdr_name_length, HEADER_Y, -10);

			xor_SetCursorPos(0, 3);
			xor_lineSpacing = false;

			xor_Print("Present System"); 
			xor_SetCursorPos(20, 3);
			xor_PrintChar(':');
			gen_PrintName(&currentSeed, true);
			xor_Print("\nHyperspace System");
			xor_SetCursorPos(20, 4);
			xor_PrintChar(':');
			gen_PrintName(&selectedSeed, true);
			xor_Print("\nCondition: ");
			xor_SetCursorPos(20, 5);
			xor_PrintChar(':');
			printPlayerCondition();

			xor_Print("\nFuel: ");
			xor_PrintUInt8Tenths(player_fuel, 1);
			xor_Print(" Light Years");
			xor_Print("\nCash: ");
			xor_PrintUInt24Tenths(player_money);
			xor_Print(" Cr");

			xor_Print("\nLegal Status: ");
			if (player_outlaw == 0) xor_Print("Clean");
			else if (player_outlaw < 50) xor_Print("Offender");
			else xor_Print("Fugitive");

			xor_Print("\nRating: ");
			if (player_kills < 4) xor_Print("Harmless");
			else if (player_kills < 8) xor_Print("Mostly Harmless");
			else if (player_kills < 16) xor_Print("Poor");
			else if (player_kills < 32) xor_Print("Average");
			else if (player_kills < 256) xor_Print("Above Average");
			else if (player_kills < 512) xor_Print("Competent");
			else if (player_kills < 2560) xor_Print("Dangerous");
			else if (player_kills < 6400) xor_Print("Deadly");
			else xor_Print("---- E L I T E ----");

			xor_Print("\n\nEQUIPMENT:");

			break;

		case THIS_DATA:
		case SEL_DATA:

			{
				const struct gen_sysData_t* relevantData = 
					currentMenu == THIS_DATA ? &thisSystemData : &selectedSystemData;
				const struct gen_seed_t* relevantSeed = 
					currentMenu == THIS_DATA ? &currentSeed : &selectedSeed;

				xor_CenterTextOffset("DATA ON", 7, HEADER_Y, 1 + strlen((*relevantData).name));
				xor_CenterTextOffset((*relevantData).name, strlen((*relevantData).name), 8, -8);

				xor_SetCursorPos(0, 6);
				xor_lineSpacing = true;

				xor_Print("Economy: "); gen_PrintEconomy(relevantData);
				xor_Print("\nGovernment: "); gen_PrintGovernment(relevantData);
				xor_Print("\nTech. Level: "); gen_PrintTechnology(relevantData);
				xor_Print("\nPopulation: "); gen_PrintPopulation(relevantData, relevantSeed);
				xor_Print("\nGross Productivity: "); gen_PrintProductivity(relevantData);
				xor_Print("\nAverage Radius: "); gen_PrintRadius(relevantSeed);
			}

			break;

		case LOCAL_MAP:

			xor_CenterText("SHORT RANGE CHART", 17, HEADER_Y);
			gen_DrawLocalMap();

			break;
	
		case GALAXY_MAP:

			xor_CenterTextOffset("GALAXY CHART", 12, HEADER_Y, 2);
			xor_CenterTextOffset((char[]){ (char)('1' + gen_currentGalaxy) }, 1, HEADER_Y, -13);
			gen_DrawGalaxyMap();
	
			break;

		case MARKET:

			xor_CenterTextOffset(thisSystemData.name, strlen(thisSystemData.name), HEADER_Y, 12);
			xor_CenterTextOffset("MARKET LINK", 11, HEADER_Y, -1 - strlen(thisSystemData.name));
			mkt_PrintMarketTable();
			xor_FillRectangle(DASH_HOFFSET + 5, 
					HEADER_DIVIDER_Y + 10 + 8 * menu_selOption, DASH_WIDTH - 10, 9);

			break;

		case INVENTORY:

			xor_CenterText(player_condition == DOCKED ? "SELL CARGO" : "CARGO HOLD", 10, HEADER_Y);

			mkt_PrintInventoryTable();

			if (player_condition == DOCKED && !mkt_InventoryEmpty())
			{
				xor_FillRectangle(DASH_HOFFSET + 5, 
						HEADER_DIVIDER_Y + 10 + 8 * menu_selOption, DASH_WIDTH - 10, 9);
			}

			break;

		case UPGRADES:

			xor_CenterText("OUTFITTING", 10, HEADER_Y);

			break;

		default:

			xor_CenterText("UNDEFINED MENU", 14, HEADER_Y);
	
	}

	gfx_BlitBuffer();
}

bool doMenuInput()
{
	clock_t frameTimer = clock();

	updateKeys();

	const enum currentMenu_t lastMenu = currentMenu;
	const unsigned char prevSelOption = menu_selOption;
	const signed int prevCrsX = gen_crsX;
	const signed int prevCrsY = gen_crsY;

	enum currentMenu_t returnMenu;

	switch (currentMenu) 
	{
		case MAIN:

			returnMenu = player_condition == DOCKED ? MARKET : NONE;

			if (up && (prevUp == 0 || prevUp > HOLD_TIME))
			{
				if (menu_selOption > 0) menu_selOption--;
				else menu_selOption = NUM_MENU_OPTIONS - 1;
			}
				
			if (down && (prevDown == 0 || prevDown > HOLD_TIME))
			{
				menu_selOption++;
				if (menu_selOption >= NUM_MENU_OPTIONS) menu_selOption = 0;
			}

			if (enter && prevEnter == 0)
			{
				switch (menu_selOption)
				{
					case 0:
						currentMenu = STATUS;
						break;
					case 1:
						currentMenu = LOCAL_MAP;
						gen_ResetCursorPosition(true);
						break;
					case 2:
						currentMenu = THIS_DATA;
						break;
					case 3:
						currentMenu = INVENTORY;
						break;
					case 5:
						toExit = true; // exit game and menu
					case 4:
						return false; // just exit menu
				}
			}

			if (menu_selOption != prevSelOption)
			{
				xor_FillRectangle(xor_clipX + LEFT_TEXT_INDENT - 2, 
						29 + 16 * menu_selOption, MM_SELBAR_WIDTH, 11);
				xor_FillRectangle(xor_clipX + LEFT_TEXT_INDENT - 2, 
						29 + 16 * prevSelOption, MM_SELBAR_WIDTH, 11);
				gfx_BlitRectangle(gfx_buffer, xor_clipX, xor_clipY, xor_clipWidth, xor_clipHeight);
			}

			break;

		case LOCAL_MAP:

			returnMenu = MAIN;

			if (left && (prevLeft == 0 || prevLeft > HOLD_TIME))
			{
				gen_crsX -= 4;
				while (gen_crsX <= -4 * LCL_MAP_DXMAX) gen_crsX++;
			}
			else if (right && (prevRight == 0 || prevRight > HOLD_TIME))
			{
				gen_crsX += 4;
				while (gen_crsX >= 4 * LCL_MAP_DXMAX) gen_crsX--;
			}
				
			if (up && (prevUp == 0 || prevUp > HOLD_TIME))
			{
				gen_crsY -= 4;
				while (gen_crsY <= -2 * LCL_MAP_DYMAX + SML_CRS_SIZE + SML_CRS_SPREAD) gen_crsY++;
			}
			else if (down && (prevDown == 0 || prevDown > HOLD_TIME))
			{
				gen_crsY += 4;
				while (gen_crsY >= 2 * LCL_MAP_DYMAX) gen_crsY--;
			}

			if (enter && prevEnter == 0) gen_SelectNearestSystem(true);

			if (graph && prevGraph == 0)
			{
				currentMenu = GALAXY_MAP;
				gen_ResetCursorPosition(false);
			}

			break;

		case GALAXY_MAP:

			returnMenu = LOCAL_MAP;

			if (left && (prevLeft == 0 || prevLeft > HOLD_TIME))
			{
				gen_crsX -= 4;
				while (gen_crsX < 0) gen_crsX++;
			}
			else if (right && (prevRight == 0 || prevRight > HOLD_TIME))
			{
				gen_crsX += 4;
				while (gen_crsX > 256) gen_crsX--;
			}
				
			if (up && (prevUp == 0 || prevUp > HOLD_TIME))
			{
				gen_crsY -= 4;
				while (gen_crsY < 0) gen_crsY++;
			}
			else if (down && (prevDown == 0 || prevDown > HOLD_TIME))
			{
				gen_crsY += 4;
				while (gen_crsY > 128) gen_crsY--;
			}

			if (enter && prevEnter == 0) gen_SelectNearestSystem(false);
			else if (yequ && prevYequ == 0) gen_ResetCursorPosition(true);
			else if (graph && prevGraph == 0) currentMenu = SEL_DATA;

			break;

		case SEL_DATA:

			returnMenu = GALAXY_MAP;

			break;

		case MARKET:

			returnMenu = UPGRADES;

			if (graph && prevGraph == 0) currentMenu = MAIN;

			if (up && (prevUp == 0 || prevUp > HOLD_TIME))
			{
				menu_selOption--;
				if (menu_selOption >= NUM_TRADE_GOODS) menu_selOption = NUM_TRADE_GOODS - 1;
			}
			else if (down && (prevDown == 0 || prevDown > HOLD_TIME))
			{
				menu_selOption++;
				if (menu_selOption >= NUM_TRADE_GOODS) menu_selOption = 0;
			}

			if (enter && prevEnter == 0)
			{
				mkt_Buy(menu_selOption);
				drawMenu(false);
			}

			if (menu_selOption != prevSelOption)
			{
				xor_FillRectangle(DASH_HOFFSET + 5, 
						HEADER_DIVIDER_Y + 10 + 8 * prevSelOption, DASH_WIDTH - 10, 9);
				xor_FillRectangle(DASH_HOFFSET + 5, 
						HEADER_DIVIDER_Y + 10 + 8 * menu_selOption, DASH_WIDTH - 10, 9);
				gfx_BlitRectangle(gfx_buffer, xor_clipX, xor_clipY, xor_clipWidth, xor_clipHeight);
			}

			break;

		case UPGRADES:

			returnMenu = UPGRADES;

			if (graph && prevGraph == 0)
			{
				currentMenu = MARKET;
			}

			break;

		case INVENTORY:

			if (player_condition == DOCKED && !mkt_InventoryEmpty())
			{
				if (up && (prevUp == 0 || prevUp > HOLD_TIME))
				{
					menu_selOption--;
					if (menu_selOption >= NUM_TRADE_GOODS) menu_selOption = NUM_TRADE_GOODS - 1;
				}
				else if (down && (prevDown == 0 || prevDown > HOLD_TIME))
				{
					menu_selOption++;
					if (menu_selOption >= NUM_TRADE_GOODS) menu_selOption = 0;
				}

				if (menu_selOption != prevSelOption)
				{
					xor_FillRectangle(DASH_HOFFSET + 5, 
							HEADER_DIVIDER_Y + 10 + 8 * prevSelOption, DASH_WIDTH - 10, 9);
					xor_FillRectangle(DASH_HOFFSET + 5, 
							HEADER_DIVIDER_Y + 10 + 8 * menu_selOption, DASH_WIDTH - 10, 9);
					gfx_BlitRectangle(gfx_buffer, xor_clipX, xor_clipY, xor_clipWidth, xor_clipHeight);
				}

				if (enter && prevEnter == 0)
				{
					mkt_Sell(menu_selOption);
					drawMenu(true);
				}
			}

		default:

			returnMenu = MAIN;
	}

	if (gen_crsX != prevCrsX || gen_crsY != prevCrsY) gen_RedrawCursorPosition(prevCrsX, prevCrsY);
	
	if (yequ && prevYequ == 0) currentMenu = returnMenu;

	updatePrevKeys();

	while (clock() - frameTimer < FRAME_TIME);

	if (currentMenu == NONE) return false;

	if (lastMenu != currentMenu) drawMenu(true);
	
	return true;
}

void begin()
{
	currentMenu = STATUS;
	player_condition = DOCKED;

	drawCycle = 0;

	originSeed = (struct gen_seed_t){ 0x5a4a, 0x0248, 0xb753 };
	currentSeed = (struct gen_seed_t){ 0xad38, 0x149c, 0x151d };

	gen_currentGalaxy = 0;

	gen_SetSystemData(&thisSystemData, &currentSeed);
	selectedSeed = currentSeed;
	selectedSystemData = thisSystemData;
	gen_ResetDistanceToTarget();
	mkt_ResetLocalMarket();

	player_dead = false;
	player_fuel = 70;
	player_money = 1000;
	player_outlaw = 0;

	player_cargo_space = 25;
	player_cargo_cap = 25;
}

unsigned char titleScreen(unsigned char shipType, char query[], unsigned char queryLength, bool acceptEnter)
{
	numShips = 0;
	NewShip(shipType, 
			(struct vector_t){ 0, 0, TTL_SHIP_START_Z }, 
			Matrix(256,0,0, 0,0,256, 0,256,0));
	ships[0].pitch = 127;
	ships[0].roll = 127;

	unsigned char returnVal = 0;

	while (true)
	{
		gfx_FillScreen(COLOR_BLACK);

		xor_CenterText("---- E L I T E ----", 19, HEADER_Y);
		xor_CenterText(query, queryLength, xor_clipY + xor_clipHeight - 3 * HEADER_Y - 8);
	
		if (ships[0].position.z > TTL_SHIP_END_Z) ships[0].position.z -= TTL_SHIP_ZOOM_RATE;
		MoveShip(0); // only rotation set, and no ai done, so it won't run away lol
		DrawShip(0);
		ships[0].orientation = orthonormalize(ships[0].orientation);

		if (!acceptEnter)
		{
			xor_SetCursorPos(6, xor_textRows);
			xor_Print("(N)");
			xor_SetCursorPos(xor_textCols - 11, xor_textRows);
			xor_Print("(Y)");
		}

		gfx_BlitBuffer();

		kb_Scan();

		if (kb_IsDown(kb_KeyClear))
		{
			numShips = 0;
			return 2;
		}

		if (!acceptEnter)
		{
			if (kb_IsDown(kb_KeyYequ))
			{
				returnVal = 0;
				break;
			}
			if (kb_IsDown(kb_KeyGraph))
			{
				returnVal = 1;
				break;
			}
		}
		else if (kb_IsDown(kb_KeyEnter)) break;
	}

	numShips = 0;
	return acceptEnter ? 1 : returnVal;
}

bool run()
{
	begin();

	unsigned char tsResponse = titleScreen(BP_COBRA, "Load Saved Commander?", 21, false);
	if (tsResponse == 2) return false; // player pressed "clear", exit the game

	else if (tsResponse == 1)
	{
		// TODO load save
	}

	// there is actually another option to exit the game here, because why not?
	if (titleScreen(BP_MAMBA, "Press ENTER to begin, Commander.", 32, true) == 2) return false;

	// core game loop. this is kinda strange, but it avoids recursion.
	// basically, we're either in a menu, or in flight, and whenever that
	// flips, it'll be to the other one. so there's actually no reason to
	// store a state! instead, we just go menu, flight, menu, flight, etc.
	while (true)
	{
		drawMenu(true);
		while (doMenuInput()); // kicks out once it's time
		if (toExit) break; // "quit" pressed instead of just "return"
		toExit = false;

		resetPlayerCondition(); // also handles launch from station if necessary
		doFlight();
		if (player_dead) break;
	}

	return player_dead;
}

int main(void)
{
	srand(rtc_Time());

	// graphics initialization
	gfx_Begin();
	gfx_SetDrawBuffer();
	gfx_SetPalette(&global_palette, sizeof_global_palette, 0);

	// run the game until the user exits
	while (run());

	// exit game
	gfx_End();
	return 0;
}
