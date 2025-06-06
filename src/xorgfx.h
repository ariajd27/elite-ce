#ifndef xorgfx_include_file
#define xorgfx_include_file

#include <stdbool.h>

bool xor_InClipRegion(signed int x, signed int y);

void xor_PointNoClip(unsigned int x, unsigned char y);
void xor_Point(unsigned int x, unsigned char y);

void xor_VerticalLine(unsigned int x, unsigned char y0, unsigned char y1);
void xor_HorizontalLine(unsigned char y, unsigned int x0, unsigned int x1);
void xor_LineNoClip(unsigned int x0, unsigned char y0, unsigned int x1, unsigned char y1);
void xor_Line(signed int x0, signed int y0, signed int x1, signed int y1);

void xor_Crosshair(unsigned int x, unsigned char y, unsigned char spread, unsigned char size);
void xor_Rectangle(unsigned int x, unsigned char y, unsigned int width, unsigned char height);
void xor_FillRectangle(signed int x, signed int y, unsigned int width, unsigned char height);
void xor_Circle(signed int cX, signed int cY, unsigned int r);
void xor_SteppedCircle(signed int cX, signed int cY, unsigned int r, unsigned char step);
void xor_FillCircle(signed int cX, signed int cY, unsigned char r);
void xor_Ellipse(signed int cX, signed int cY, signed int uX, signed int uY, signed int vX, signed int vY, unsigned char end);

void xor_CenterText(char toPrint[], unsigned char length, unsigned char y);
void xor_CenterTextOffset(const char toPrint[], unsigned char length, unsigned char y, signed char offset);

extern bool xor_lineSpacing;
extern unsigned char xor_cursorX;
extern unsigned char xor_cursorY;

void xor_SetCursorPos(unsigned char x, unsigned char y);
void xor_PrintChar(char toPrint);
void xor_Print(char const str[]);
void xor_PrintUInt8(unsigned char toPrint, unsigned char maxLength);
void xor_PrintUInt8Tenths(unsigned char toPrint, unsigned char maxLength);
void xor_PrintUInt24(unsigned int toPrint, unsigned char maxLength);
void xor_PrintUInt24Adaptive(unsigned int toPrint);
void xor_PrintUInt24Tenths(unsigned int toPrint);

#endif
