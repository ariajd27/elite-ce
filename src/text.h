#ifndef text_include_file
#define text_include_file

extern unsigned char txt_lineSpacing;
extern unsigned char txt_cursorX;
extern unsigned char txt_cursorY;

extern const char twokens[32][2];
extern const char recursiveTokens[955];

void txt_SetCursorPos(const unsigned char row, const unsigned char col);
void txt_LF();
void txt_CRLF();

void txt_PutC(const char c);
void txt_PutTok(const char c);
void txt_PutTokColon(const char c);
void txt_PutRecursive(char c);
void txt_PutString(char *c);
void txt_PutUInt32(unsigned int n, unsigned char width, bool tenths);

#endif
