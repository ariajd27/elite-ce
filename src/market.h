#ifndef market_include_file
#define market_include_file

#include "variables.h"

extern unsigned char marketSeed;
extern unsigned char mkt_localQuantities[NUM_TRADE_GOODS];
extern unsigned char inventory[NUM_TRADE_GOODS];

void mkt_ResetLocalMarket();
void mkt_PrintMarketTable();
void mkt_PrintInventoryTable();

// queries the amount then makes the transaction, updating inventory & money
bool mkt_Buy(unsigned char goodIndex);
bool mkt_Sell(unsigned char crsPos);

bool mkt_InventoryEmpty();

void mkt_AdjustLegalStatus();

#endif
