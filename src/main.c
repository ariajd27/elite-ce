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

#include <debug.h>

bool toExit = false;
unsigned char drawCycle;

enum {
	DOCKED,
	GREEN,
	YELLOW,
	RED
} player_condition;

enum currentMenu_t currentMenu;
unsigned char menu_selOption = 0;

unsigned char player_speed;
signed char player_acceleration;
signed char player_roll;
signed char player_pitch;

unsigned char player_fuel;
unsigned int player_money;

unsigned char player_cargo_space;
unsigned char player_cargo_cap;

enum viewDirMode_t viewDirMode;

bool yequ, graph, up, down, left, right, enter;
unsigned char prevYequ, prevGraph, prevUp, prevDown, prevLeft, prevRight, prevEnter;

void drawSpaceView()
{
	if (viewDirMode == FRONT) 
		xor_CenterText("Front View", 10, 10);
	else if (viewDirMode == LEFT)
		xor_CenterText("Left View", 9, 10);
	else if (viewDirMode == RIGHT)
		xor_CenterText("Right View", 10, 10);
	else xor_CenterText("Rear View", 9, 10);

	xor_Crosshair(VIEW_HCENTER, VIEW_VCENTER, CRS_SPREAD, CRS_SIZE);

	// ships
	FlipAxes(viewDirMode);
	for(unsigned char i = 0; i < numShips; i++) DrawShip(i);
	RestoreAxes(viewDirMode);

	stardust_Move(viewDirMode, player_speed, player_pitch, player_roll);
	stardust_Draw(); // order is not really important here
}

void drawDashboard()
{
	// sprited sections
	gfx_Sprite(dashleft, DASH_HOFFSET, DASH_VOFFSET);
	gfx_Sprite(dashcenter, DASH_HOFFSET_CENTER, DASH_VOFFSET);
	gfx_Sprite(dashright, DASH_HOFFSET_RIGHT, DASH_VOFFSET);

	// radar dots
	for (unsigned char i = 0; i < numShips; i++)
	{
		// check to make sure ship in range
		if (ships[i].position.x < -63 * 256) continue;
		if (ships[i].position.x > 63 * 256) continue;
		if (ships[i].position.y < -63 * 256) continue;
		if (ships[i].position.y > 63 * 256) continue;
		if (ships[i].position.z < -63 * 256) continue;
		if (ships[i].position.z > 63 * 256) continue;

		unsigned int const x = RADAR_HCENTER + ships[i].position.x / RADAR_XSCALE;
		unsigned char const y = RADAR_VCENTER - ships[i].position.z / RADAR_ZSCALE;
		signed char dy = ships[i].position.y / RADAR_YSCALE;

		// clip to fit in center dash segment
		if (y + dy < DASH_VOFFSET + 2)
		{
			dy += (DASH_VOFFSET + 3) - (y + dy);
		}
		else if (y + dy > GFX_LCD_HEIGHT - 2)
		{
			dy -= (y + dy) - (GFX_LCD_HEIGHT - 3);
		}

		gfx_SetColor(COLOR_YELLOW);
		gfx_Line(x, y, x, y + dy);
		gfx_SetPixel(x, y + dy);
		gfx_SetPixel(x - 1, y + dy);
	}

	// right panel
	// speed display
	gfx_SetColor(COLOR_YELLOW);
	unsigned char const speedBarLength = 32 * player_speed / PLAYER_MAX_SPEED;
	gfx_FillRectangle(DASH_HOFFSET_RIGHT + 4, DASH_VOFFSET + 2, speedBarLength, 3);

	// roll display
	signed char rollBarOffset = player_roll / 2;
	if (rollBarOffset == 16) rollBarOffset--;
	else if (rollBarOffset == -16) rollBarOffset++;
	gfx_FillRectangle(DASH_HOFFSET_RIGHT + 20 + rollBarOffset, DASH_VOFFSET + 10, 2, 3);

	// pitch display
	signed char pitchBarOffset = player_pitch * 2;
	if (pitchBarOffset == 16) pitchBarOffset--;
	else if (pitchBarOffset == -16) pitchBarOffset++;
	gfx_FillRectangle(DASH_HOFFSET_RIGHT + 20 + pitchBarOffset, DASH_VOFFSET + 18, 2, 3);
}

void updateKeys()
{
	kb_Scan();

	yequ = kb_IsDown(kb_KeyYequ);
	graph = kb_IsDown(kb_KeyGraph);
	up = kb_IsDown(kb_KeyUp);
	down = kb_IsDown(kb_KeyDown);
	left = kb_IsDown(kb_KeyLeft);
	right = kb_IsDown(kb_KeyRight);
	enter = kb_IsDown(kb_KeyEnter);
}

void updatePrevKeys()
{
	prevYequ = yequ ? prevYequ + 1 : 0;
	prevGraph = graph ? prevGraph + 1 : 0;
	prevUp = up ? prevUp + 1 : 0;
	prevDown = down ? prevDown + 1 : 0;
	prevLeft = left ? prevLeft + 1 : 0;
	prevRight = right ? prevRight + 1 : 0;
	prevEnter = enter ? prevEnter + 1 : 0;
}

bool doFlightInput()
{
	dbg_printf("doing flight input...\n");

	updateKeys();

	// acceleration
	if (kb_IsDown(kb_Key2nd)) player_acceleration = 3;
	else if (kb_IsDown(kb_KeyAlpha)) player_acceleration = -1;
	
	if (!yequ) // pitch/roll controls
	{
		// roll
		if (left) player_roll -= 4;
		else if (right) player_roll += 4;

		// damping
		else if (-1 <= player_roll && player_roll <= 1) player_roll = 0; 
		else if (player_roll < 0) player_roll += 2;
		else if (player_roll > 0) player_roll -= 2;

		// clamping
		if (player_roll > 31) player_roll = 31;
		else if (player_roll < -31) player_roll = -31;
	
		// pitch
		if (up) player_pitch++;
		else if (down) player_pitch--;

		// damping
		else if (player_pitch < 0) player_pitch++;
		else if (player_pitch > 0) player_pitch--;

		// clamping
		if (player_pitch > 7) player_pitch = 7;
		else if (player_pitch < -7) player_pitch = -7;
	}
	else // view switching
	{
		if (up) viewDirMode = FRONT;
		else if (left) viewDirMode = LEFT;
		else if (right) viewDirMode = RIGHT;
		else if (down) viewDirMode = REAR;

		// still need to do pitch/roll damping
		if (player_pitch < 0) player_pitch++;
		else if (player_pitch > 0) player_pitch--;
		if (player_roll < 0) player_roll++;
		else if (player_roll > 0) player_roll--;	
	}

	updatePrevKeys();

	if (graph && prevGraph == 1) // 1 bc we are after 1 update... kinda janky, but...
	{
		currentMenu = MAIN;
		return false;
	}
	else return true;
}

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
			xor_Print("\nRating: ");
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
	dbg_printf("preparing initial gamestate...\n");
	
	currentMenu = STATUS;
	player_condition = DOCKED;

	drawCycle = 0;

	originSeed = (struct gen_seed_t){ 0x5a4a, 0x0248, 0xb753 };
	currentSeed = originSeed;

	gen_currentGalaxy = 0;
	gen_SetSystemData(&thisSystemData, &currentSeed);
	selectedSeed = currentSeed;
	selectedSystemData = thisSystemData;
	gen_ResetDistanceToTarget();
	mkt_ResetLocalMarket();

	player_speed = 0;
	player_acceleration = 0;
	player_roll = 0;
	player_pitch = 0;

	player_fuel = 70;
	player_money = 1000;

	player_cargo_space = 25;
	player_cargo_cap = 25;

	viewDirMode = FRONT;
}

void doFlight()
{
	dbg_printf("doing flight...\n");

	while (doFlightInput())
	{
		clock_t frameTimer = clock();

		// black background
		// gfx_SetColor(COLOR_BLACK);
		// gfx_FillRectangle(0, 0, GFX_LCD_WIDTH, GFX_LCD_HEIGHT);	
		gfx_FillScreen(COLOR_BLACK);

		// outer white frame
		gfx_SetColor(COLOR_WHITE);
		gfx_Rectangle(DASH_HOFFSET, 0, GFX_LCD_WIDTH - 2 * DASH_HOFFSET, GFX_LCD_HEIGHT - dashleft_height);

		// apply acceleration to speed
		if (player_acceleration >= 0 || player_speed >= -1 * player_acceleration) 
		{
			player_speed += player_acceleration;
		}
		if (player_speed > PLAYER_MAX_SPEED)
		{
			player_speed = PLAYER_MAX_SPEED;
		}
		player_acceleration = 0;
	
		// tidy vectors for each ship -- one ship per cycle
		ships[drawCycle % MAX_SHIPS].orientation = orthonormalize(ships[drawCycle % MAX_SHIPS].orientation);

		// do ai -- two ships per cycle
		DoAI(drawCycle % MAX_SHIPS);
		DoAI((drawCycle + MAX_SHIPS / 2) % MAX_SHIPS);
	
		for (unsigned char i = 0; i < numShips; i++)
		{
			// apply speed to other ships
			ships[i].position.z -= player_speed;
	
			// apply pitch and roll to other ships' positions
			signed int oldY = ships[i].position.y - (player_roll * ships[i].position.x) / 256;
			ships[i].position.z += (player_pitch * oldY) / 256;
			ships[i].position.y = oldY - (player_pitch * ships[i].position.z) / 256;
			ships[i].position.x += (player_roll * ships[i].position.y) / 256;
	
			// apply pitch and roll to other ships' orientations
			for (unsigned char j = 0; j < 9; j += 3)
			{
				ships[i].orientation.a[j + 1] -= player_roll * ships[i].orientation.a[j + 0] / 256;
				ships[i].orientation.a[j + 0] += player_roll * ships[i].orientation.a[j + 1] / 256;
				ships[i].orientation.a[j + 1] -= player_pitch * ships[i].orientation.a[j + 2] / 256;
				ships[i].orientation.a[j + 2] += player_pitch * ships[i].orientation.a[j + 1] / 256;
			}
	
			MoveShip(i);
		}

		drawSpaceView();
		drawCycle++;

		drawDashboard();

		dbg_printf("frame ready...\n");

		while (clock() - frameTimer < FRAME_TIME);

		gfx_BlitBuffer();
		
		dbg_printf("blit complete\n");
	}
}

void resetPlayerCondition()
{
	player_condition = GREEN;
}

bool run()
{
	begin();	

	// core game loop
	while (true)
	{
		drawMenu(true);
		while (doMenuInput()); // kicks out once it's time
		if (toExit) break;

		resetPlayerCondition();
		doFlight(); // no assurances here--- changed a bit during menu overhaul.
					// i haven't tested yet if the loop still loops properly
	}

	return false;
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
