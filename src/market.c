#include <stdlib.h>
#include <stdbool.h>
#include <keypadc.h>
#include <time.h>

#include "market.h"
#include "generation.h"
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
	{  40, -5, 't', 226, 0x1f },
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
	{  53, 15, 't', 192, 0x07 }
};

const char productNames[NUM_TRADE_GOODS][13] = {
	"Food",
	"Textiles",
	"Radioactives",
	"Slaves",
	"Liquor/Wines",
	"Luxuries",
	"Narcotics",
	"Computers",
	"Machinery",
	"Alloys",
	"Firearms",
	"Furs",
	"Minerals",
	"Gold",
	"Platinum",
	"Gemstones",
	"Alien Items"
};

struct mkt_localEntry_t {
	unsigned char price;
	unsigned char quantity;
} localEntries[NUM_TRADE_GOODS];

unsigned char inventory[NUM_TRADE_GOODS];

void mkt_ResetLocalMarket()
{
	const unsigned char r = rand() % 256;

	for (unsigned char i = 0; i < NUM_TRADE_GOODS; i++)
	{
		localEntries[i].price = 
			tradeGoods[i].basePrice 
			+ (r & tradeGoods[i].mask) 
			+ thisSystemData.economy * tradeGoods[i].econFactor;

		const signed int tempQuantity =
			tradeGoods[i].baseQuantity
			+ (r & tradeGoods[i].mask)
			- thisSystemData.economy * tradeGoods[i].econFactor;
		localEntries[i].quantity = tempQuantity < 0 ? 0 : tempQuantity % 64;
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

		xor_SetCursorPos(15, y);
		xor_PrintUInt24(localEntries[i].price * 4 / 10, 2);
		xor_PrintChar('.');
		xor_PrintUInt8(localEntries[i].price * 4 % 10, 1);
		xor_Print(" Cr");

		if (localEntries[i].quantity > 0)
		{
			xor_SetCursorPos(24, y);
			xor_PrintUInt8(localEntries[i].quantity, 2);
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

#include <debug.h>

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

	if (quantity > localEntries[goodIndex].quantity) return false; // not enough to buy
	if (quantity < player_cargo_space) return false; // not enough space in hold

	unsigned int price = (unsigned int)localEntries[goodIndex].price * quantity * 4;
	if (price <= player_money) 
	{
		inventory[goodIndex] += quantity;
		localEntries[goodIndex].quantity -= quantity;
		player_money -= price;
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

	player_money += (unsigned int)localEntries[goodIndex].price * quantity * 4;
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
