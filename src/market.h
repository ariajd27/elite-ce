#ifndef market_include_file
#define market_include_file

void mkt_ResetLocalMarket();
void mkt_PrintMarketTable();
void mkt_PrintInventoryTable();

// queries the amount then makes the transaction, updating inventory & money
bool mkt_Buy(unsigned char goodIndex);
bool mkt_Sell(unsigned char goodIndex);

#endif
