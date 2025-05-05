#include <stdlib.h>
#include <stdbool.h>
#include <keypadc.h>
#include <time.h>

#include "market.h"
#include "generation.h"
#include "upgrades.h"
#include "xorgfx.h"
#include "variables.h"

const struct mkt_tradeGood_t {
	unsigned char basePrice;
	signed char econFactor;
	char unitSymbol;
	unsigned char baseQuantity;
	unsigned char mask;
} tradeGoods[NUM_TRADE_GOODS] = {
	{  19, -2, 't',   6, 0x01 },
	{  20, -1, 't',  10, 0x03 },
	{  65, -3, 't',   2, 0x07 },
	{  40, 11, 't', 226, 0x07 }, // modified... explanation below...
	{  83, -5, 't', 251, 0x0f },
	{ 196,  8, 't',  54, 0x03 },
	{ 235, 29, 't',   8, 0x78 },
	{ 154, 14, 't',  56, 0x03 },
	{ 117,  6, 't',  40, 0x07 },
	{  78,  1, 't',  17, 0x1f },
	{ 124, 13, 't',  29, 0x07 },
	{ 176, -9, 't', 220, 0x3f },
	{  32, -1, 't',  53, 0x03 },
	{  97, -1, 'k',  66, 0x07 },
	{ 171, -2, 'k',  55, 0x1f },
	{  45, -1, 'g', 250, 0x0f },
	{  53, 15, 'g', 192, 0x07 }
};

const char productNames[NUM_TRADE_GOODS][13] = {
	"Food",
	"Textiles",
	"Radioactives",
	"Chemicals", 		// <- these are slaves in the original. it took a lot of deliberation,
	"Liquor/Wines",		// but yes, i decided to remove the slave trade. ultimately, it's
	"Luxuries",			// just too *weird* to distribute a game *in 2024* where the player
	"Narcotics",		// can be a *slave trader* and be rewarded for it! like, *seriously?*
	"Computers",		// i don't think they add nearly enough to *Elite* to justify releasing
	"Machinery",		// a slave trade simulator where human beings are measured by the ton.
	"Alloys",
	"Firearms",
	"Furs",
	"Minerals",
	"Gold",
	"Platinum",
	"Gemstones",
	"Alien Items"
};

unsigned char mkt_localPrices[NUM_TRADE_GOODS];
unsigned char mkt_localQuantities[NUM_TRADE_GOODS];
unsigned char inventory[NUM_TRADE_GOODS];

unsigned char marketSeed;

void mkt_ResetLocalMarket()
{
	for (unsigned char i = 0; i < NUM_TRADE_GOODS; i++)
	{
		mkt_localPrices[i] = 
			tradeGoods[i].basePrice 
			+ (marketSeed & tradeGoods[i].mask) 
			+ thisSystemData.economy * tradeGoods[i].econFactor;

		const signed int tempQuantity =
			tradeGoods[i].baseQuantity
			+ (marketSeed & tradeGoods[i].mask)
			- thisSystemData.economy * tradeGoods[i].econFactor;
		mkt_localQuantities[i] = tempQuantity < 0 ? 0 : tempQuantity % 64;
	}
}

void mkt_PrintMarketTable()
{
	// headings
	xor_SetCursorPos(1, 3);
	xor_Print("PRODUCT");
	xor_SetCursorPos(16, 3);
	xor_Print("PRICE");
	xor_SetCursorPos(23, 3);
	xor_Print("AMOUNT");

	// table body
	for (unsigned char i = 0; i < NUM_TRADE_GOODS; i++)
	{
		const unsigned char y = i + 4;

		xor_SetCursorPos(0, y);
		xor_Print(productNames[i]);

		xor_SetCursorPos(14, y);
		xor_PrintUInt24(mkt_localPrices[i] * 4 / 10, 3);
		xor_PrintChar('.');
		xor_PrintUInt8(mkt_localPrices[i] * 4 % 10, 1);
		xor_Print(" Cr");

		if (mkt_localQuantities[i] > 0)
		{
			xor_SetCursorPos(24, y);
			xor_PrintUInt8(mkt_localQuantities[i], 2);
			xor_PrintChar(tradeGoods[i].unitSymbol);
			if (tradeGoods[i].unitSymbol == 'k') xor_PrintChar('g');
		}
		else
		{
			xor_SetCursorPos(25, y);
			xor_PrintChar('-');
		}
	}
}

void mkt_PrintInventoryTable()
{
	const unsigned char cargoCap = player_upgrades.largeCargoBay ? 35 : 25;

	// capacity
	xor_SetCursorPos(24, 1);
	xor_PrintUInt8(cargoCap - player_cargo_space, 2);
	xor_PrintChar('/');
	xor_PrintUInt8(cargoCap, 2);
	xor_PrintChar('t');

	// headings
	xor_SetCursorPos(1, 3);
	xor_Print("PRODUCT");
	xor_SetCursorPos(23, 3);
	xor_Print("AMOUNT");

	unsigned char y = 4;
	for (unsigned char i = 0; i < NUM_TRADE_GOODS; i++)
	{
		if (inventory[i] == 0) continue;

		xor_SetCursorPos(0, y);
		xor_Print(productNames[i]);
		xor_SetCursorPos(24, y);
		xor_PrintUInt8(inventory[i], 2);
		xor_PrintChar(tradeGoods[i].unitSymbol);
		if (tradeGoods[i].unitSymbol == 'k') xor_PrintChar('g');

		y++;
	}
}

void mkt_PrintQtyQuery(unsigned char const goodIndex, bool const toBuy)
{
	xor_SetCursorPos(0, NUM_TRADE_GOODS + 4);
	xor_Print("Qty ");
	xor_Print(productNames[goodIndex]);
	xor_Print(" to ");
	xor_Print(toBuy ? "buy" : "sell");
	xor_Print("? ");
}

void mkt_PrintQueryQty(unsigned char const x, unsigned char const quantity)
{
	xor_SetCursorPos(x, NUM_TRADE_GOODS + 4);
	xor_PrintUInt24Adaptive(quantity);
}

unsigned char mkt_AskForQuantity(unsigned char goodIndex, bool toBuy)
{
	mkt_PrintQtyQuery(goodIndex, toBuy);
	const unsigned char resetX = xor_cursorX;
	mkt_PrintQueryQty(resetX, 0);
	gfx_BlitBuffer();

	unsigned int quantity = 0;
	bool prev[10];
	bool current[10];
	bool prevEnter = kb_IsDown(kb_KeyEnter);
	while (true)
	{
		clock_t frameTimer = clock();

		kb_Scan();

		if (kb_IsDown(kb_KeyClear)) 
		{
			mkt_PrintQtyQuery(goodIndex, toBuy);
			mkt_PrintQueryQty(resetX, quantity);
			return 0;
		}

		current[0] = kb_IsDown(kb_Key0);
		current[1] = kb_IsDown(kb_Key1);
		current[2] = kb_IsDown(kb_Key2);
		current[3] = kb_IsDown(kb_Key3);
		current[4] = kb_IsDown(kb_Key4);
		current[5] = kb_IsDown(kb_Key5);
		current[6] = kb_IsDown(kb_Key6);
		current[7] = kb_IsDown(kb_Key7);
		current[8] = kb_IsDown(kb_Key8);
		current[9] = kb_IsDown(kb_Key9);

		unsigned char prevQty = quantity;

		for (unsigned char i = 0; i < 10; i++) 
		{
			if (current[i] && !prev[i])
			{ 
				quantity *= 10;
				quantity += i;
			}

			prev[i] = current[i];
		}

		if (quantity > 255) quantity = prevQty;

		// rewrite new display number
		mkt_PrintQueryQty(resetX, prevQty);
		mkt_PrintQueryQty(resetX, quantity);

		while (clock() - frameTimer < FRAME_TIME);

		gfx_BlitRectangle(gfx_buffer, xor_clipX, xor_clipY, xor_clipWidth, xor_clipHeight);

		if (kb_IsDown(kb_KeyEnter) && !prevEnter) break;
		prevEnter = kb_IsDown(kb_KeyEnter);
	}

	return quantity;
}

bool mkt_Buy(unsigned char goodIndex)
{
	unsigned char const quantity = mkt_AskForQuantity(goodIndex, true);

	if (quantity > mkt_localQuantities[goodIndex]) return false; // not enough to buy
	
	// check if sufficient cargo hold space available
	if (tradeGoods[goodIndex].unitSymbol == 't')
	{
		if (quantity > player_cargo_space) return false;
	}
	else
	{
		if (inventory[goodIndex] + quantity > 200) return false;
	}

	unsigned int price = (unsigned int)mkt_localPrices[goodIndex] * quantity * 4;
	if (price <= player_money) 
	{
		inventory[goodIndex] += quantity;
		mkt_localQuantities[goodIndex] -= quantity;
		player_money -= price;
		if (tradeGoods[goodIndex].unitSymbol == 't') player_cargo_space -= quantity;
		return true;
	}
	else return false; // can't afford
}

bool mkt_Sell(unsigned char crsPos)
{
	unsigned char goodIndex;
	unsigned char toGo = crsPos + 1;
	for (unsigned char i = 0; true; i++)
	{
		if (inventory[i] > 0) toGo--;
		if (toGo == 0)
		{
			goodIndex = i;
			break;
		}
	}

	unsigned char const quantity = mkt_AskForQuantity(goodIndex, false);

	if (quantity > inventory[goodIndex]) return false; // not enough to sell

	player_money += (unsigned int)mkt_localPrices[goodIndex] * quantity * 4;
	if (tradeGoods[goodIndex].unitSymbol == 't') player_cargo_space += quantity;
	inventory[goodIndex] -= quantity;
	return true;
}

bool mkt_InventoryEmpty()
{
	for (unsigned char i = 0; i < NUM_TRADE_GOODS; i++)
	{
		if (inventory[i] > 0) return false;
	}

	return true;
}

void mkt_AdjustLegalStatus()
{
				  // 2 *   narcotics  +   firearms
	player_outlaw |= 2 * inventory[6] + inventory[10];
}

void mkt_GetScanned()
{
	player_outlaw |= 2 * (2 * inventory[6] + inventory[10]);
}
