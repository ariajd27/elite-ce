#ifndef text_include_file
#define text_include_file

extern unsigned char txt_lineSpacing;
extern unsigned char txt_cursorX;
extern unsigned char txt_cursorY;

extern const char twokens[32][2];
extern const char recursiveTokens[955];

void txt_SetCursorRow(const unsigned char row);
void txt_SetCursorCol(const unsigned char col);
void txt_SetCursorPos(const unsigned char row, const unsigned char col);
void txt_LF();
void txt_CRLF();

void txt_PutC(const unsigned char c);
void txt_PutTok(const unsigned char c);
void txt_PutRecursive(unsigned char c);
void txt_PutString(unsigned char *c);
void txt_PutUInt32(unsigned long n, unsigned char width, bool tenths);

void txt_PutTokSpace(const unsigned char c);
void txt_PutTokColon(const unsigned char c);
void txt_PutTokCRLF(const unsigned char c);
void txt_PutTokCRLFTab(const unsigned char c);

#endif
