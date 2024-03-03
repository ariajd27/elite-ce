#include <graphx.h>
#include <stdlib.h>
#include <stdbool.h>
#include <debug.h>

#include "xorgfx.h"
#include "intmath.h"
#include "trig.h"
#include "font_data.h"
#include "variables.h"

unsigned char xor_cursorX = 0;
unsigned char xor_cursorY = 0;

bool xor_lineSpacing;

bool xor_InClipRegion(signed int x, signed int y)
{
	if (x < xor_clipX) return 0;
	if (y < xor_clipY) return 0;
	if (x >= xor_clipX + xor_clipWidth) return 0;
	if (y >= xor_clipY + xor_clipHeight) return 0;

	return 1;
}

void xor_PointNoClip(unsigned int x, unsigned char y)
{
	gfx_vbuffer[y][x] ^= 1;
}

void xor_Point(unsigned int x, unsigned char y)
{
	if (x < xor_clipX) return;
	if (y < xor_clipY) return;
	if (x >= xor_clipX + xor_clipWidth) return;
	if (y >= xor_clipY + xor_clipHeight) return;

	gfx_vbuffer[y][x] ^= 1;
}

void xor_VerticalLine(unsigned int x, unsigned char y0, unsigned char y1)
{
	if (y0 > y1)
	{
		xor_VerticalLine(x, y1, y0);
		return;
	}

	for (unsigned char y = y0; y <= y1; y++) xor_PointNoClip(x, y);
}

void xor_HorizontalLine(unsigned char y, unsigned int x0, unsigned int x1)
{
	if (x0 > x1)
	{
		xor_HorizontalLine(y, x1, x0);
		return;
	}

	for (unsigned int x = x0; x <= x1; x++) xor_PointNoClip(x, y);
}

void xor_LineLow(unsigned int x0, unsigned char y0, unsigned int x1, unsigned char y1)
{
	const int dx = x1 - x0;
	int dy = y1 - y0;	
	char yi = 1;
	if (dy < 0)
	{
		yi = -1;
		dy = -dy;
	}

	int D = 2 * dy - dx;
	unsigned char y = y0;
	for (unsigned int x = x0; x <= x1 && x < xor_clipX + xor_clipWidth; x++)
	{
		xor_PointNoClip(x, y);

		if (D > 0)
		{
			y += yi;
			D += 2 * (dy - dx);
		}
		else 
		{
			D += 2 * dy;
		}
	}
}

void xor_LineHigh(unsigned int x0, unsigned char y0, unsigned int x1, unsigned char y1)
{
	int dx = x1 - x0;
	const int dy = y1 - y0;

	int xi = 1;
	if (dx < 0)
	{
		xi = -1;
		dx = -dx;
	}

	int D = 2 * dx - dy;
	unsigned int x = x0;
	for (unsigned char y = y0; y <= y1 && y < xor_clipY + xor_clipHeight; y++)
	{
		xor_PointNoClip(x, y);

		if (D > 0)
		{
			x += xi;
			D += 2 * (dx - dy);
		}
		else
		{
			D += 2 * dx;
		}
	}
}

void xor_LineNoClip(unsigned int x0, unsigned char y0, unsigned int x1, unsigned char y1)
{
	if (x0 == x1)
	{
		xor_VerticalLine(x0, y0, y1);
		return;
	}
	if (y0 == y1)
	{
		xor_HorizontalLine(y0, x0, x1);
		return;
	}

	unsigned char dx = (x1 > x0) ? x1 - x0 : x0 - x1;
	unsigned char dy = (y1 > y0) ? y1 - y0 : y0 - y1;

	if (dy < dx)
	{
		if (x0 > x1) xor_LineLow(x1, y1, x0, y0);
		else xor_LineLow(x0, y0, x1, y1);
	}
	else
	{
		if (y0 > y1) xor_LineHigh(x1, y1, x0, y0);
		else xor_LineHigh(x0, y0, x1, y1);
	}
}

void xor_Line(signed int x0, signed int y0, signed int x1, signed int y1)
{
	signed int const xor_clipX1 = xor_clipX + xor_clipWidth;
	signed int const xor_clipY1 = xor_clipY + xor_clipHeight;

	signed int const dx = (x1 - x0);
	signed int const dy = (y1 - y0);

	// throw out lines that are way offscreen
	if (x0 - xor_clipWidth >= xor_clipX1 && x1 - xor_clipWidth >= xor_clipX1) return;
	if (x0 + xor_clipWidth < xor_clipX && x1 + xor_clipWidth < xor_clipX) return;
	if (y0 - xor_clipHeight >= xor_clipY1 && y1 - xor_clipHeight >= xor_clipY1) return;
	if (y0 + xor_clipHeight < xor_clipY && y1 + xor_clipHeight < xor_clipY) return;

	if (x0 < xor_clipX)
	{
		if (x1 < xor_clipX) return; // line starts and ends left of viewport

		y0 += (xor_clipX - x0) * dy / dx;
		x0 = xor_clipX;
	}
	else if (x0 >= xor_clipX1)
	{
		if (x1 >= xor_clipX1) return; // line entirely right of viewport
		
		y0 -= (x0 - xor_clipX1) * dy / dx;
		x0 = xor_clipX + xor_clipWidth - 1;
	}

	if (y0 < xor_clipY)
	{
		if (y1 < xor_clipY) return; // line entirely above viewport

		x0 += (xor_clipY - y0) * dx / dy;
		y0 = xor_clipY;
	}
	else if (y0 >= xor_clipY1)
	{
		if (y1 >= xor_clipY1) return; // line entirely below viewport

		x0 -= (y0 - xor_clipY1) * dx / dy;
		y0 = xor_clipY + xor_clipHeight - 1;
	}

	if (x1 >= xor_clipX && x1 < xor_clipX1 && y1 >= xor_clipY && y1 < xor_clipY1)
	{
		xor_LineNoClip(x0, y0, x1, y1);
	}
	else
	{
		xor_Line(x1, y1, x0, y0); // this will clip the other point
	}
}

void xor_Crosshair(unsigned int x, unsigned char y, unsigned char spread, unsigned char size)
{
	for (unsigned char i = spread; i < spread + size; i++)
	{
		xor_Point(x, y + i);
		xor_Point(x, y - i);
		xor_Point(x + i, y);
		xor_Point(x - i, y);
	}
}

void xor_Rectangle(unsigned int x, unsigned char y, unsigned int width, unsigned char height)
{
	xor_HorizontalLine(y, x, x + width);
	xor_HorizontalLine(y + height, x, x + width);
	xor_VerticalLine(x, y + 1, y + height - 1);
	xor_VerticalLine(x + width, y + 1, y + height - 1);
}

void xor_FillRectangle(unsigned int x, unsigned char y, unsigned int width, unsigned char height)
{
	if (x < xor_clipX)
	{
		if (x + width < xor_clipX) return;
		xor_FillRectangle(xor_clipX, y, width - (xor_clipX - x), height);
		return;
	}

	if (x + width >= xor_clipX + xor_clipWidth)
	{
		if (x >= xor_clipX + xor_clipWidth) return;
		xor_FillRectangle(x, y, xor_clipX + xor_clipWidth - x - 1, height);
		return;
	}

	if (y < xor_clipY)
	{
		if (y + height < xor_clipY) return;
		xor_FillRectangle(x, xor_clipY, width, height - (xor_clipY - y));
		return;
	}

	if (y + height >= xor_clipY + xor_clipHeight)
	{
		if (y >= xor_clipY + xor_clipHeight) return;
		xor_FillRectangle(x, y, width, xor_clipY + xor_clipHeight - y - 1);
		return;
	}

	// only get here if no clipping needed, so noclip line routine is ok (hopefully!)
	for (signed int yy = y; yy < y + height; yy++) 
	{
		xor_HorizontalLine(yy, x, x + width - 1);
	}
}

void xor_Circle(signed int cX, signed int cY, unsigned int r)
{
	if (r < 8) xor_SteppedCircle(cX, cY, r, 8);
	else if (r < 60) xor_SteppedCircle(cX, cY, r, 4);
	else xor_SteppedCircle(cX, cY, r, 2);
}

void xor_SteppedCircle(signed int cX, signed int cY, unsigned int r, unsigned char step)
{
	if (cX + r < xor_clipX) return;
	if (cX - r >= xor_clipX + xor_clipWidth) return;
	if (cY + r < xor_clipY) return;
	if (cY - r >= xor_clipY + xor_clipHeight) return;

	signed int lastX = cX + r - 1;
	signed int lastY = cY;

	for (unsigned char i = 0; i < 64; i += step)
	{
		const signed int newX = cX + trig_cos(i) * (signed int)r / 256;
		const signed int newY = cY - trig_sin(i) * (signed int)r / 256;

		if (i != 0) xor_Point(newX, newY); // inefficiently beating the XOR logic
		xor_Line(lastX, lastY, newX, newY);

		lastX = newX;
		lastY = newY;
	}

	xor_Line(lastX, lastY, cX + r - 1, cY);
}

void xor_FillCircle(signed int cX, signed int cY, unsigned char r)
{
	const unsigned char fringeSize = r >= 96 ? 8 
								   : r >= 40 ? 4
								   : r >= 16 ? 2
								   : 0;

	unsigned int radiusSquared = r * r;

	for (signed int yy = -1 * r; yy <= r; yy++)
	{
		if (cY + yy < xor_clipY) continue;
		if (cY + yy >= xor_clipY + xor_clipHeight) continue;

		unsigned char thisRadius = intsqrt(radiusSquared - yy * yy);
		if (fringeSize > 0) thisRadius += rand() % fringeSize;
		
		signed int thisLeft = cX - thisRadius;
		if (thisLeft >= xor_clipX + xor_clipWidth) continue;
		if (thisLeft < xor_clipX) thisLeft = xor_clipX;

		signed int thisRight = cX + thisRadius;
		if (thisRight < xor_clipX) continue;
		if (thisRight >= xor_clipX + xor_clipWidth) thisRight = xor_clipX + xor_clipWidth - 1;

		xor_HorizontalLine(cY + yy, thisLeft, thisRight);
	}
}

void xor_Char(unsigned int x, unsigned char y, char toPrint)
{
	const unsigned char* charData = font_data + (toPrint - 32) * 8;

	for (unsigned char yy = 0; yy < 8; yy++)
	{
		for (unsigned char xx = 0; xx < 8; xx++)
		{
			if ((charData[yy] & (0x80 >> xx)) == 0) continue;
			xor_PointNoClip(x + xx, y + yy);
		}
	}
}

void xor_CenterText(char toPrint[], unsigned char length, unsigned char y)
{
	for (unsigned int x = 0; x < length; x++)
	{
		xor_Char(VIEW_HCENTER - 4 * length + 8 * x, y, toPrint[x]);
	}
}

void xor_CenterTextOffset(const char toPrint[], unsigned char length, unsigned char y, signed char offset)
{
	for (unsigned int x = 0; x < length; x++)
	{
		xor_Char(VIEW_HCENTER - 4 * length - 4 * offset + 8 * x, y, toPrint[x]);
	}
}

void xor_SetCursorPos(unsigned char x, unsigned char y)
{
	xor_cursorX = x;
	xor_cursorY = y;
}

void xor_CRLF()
{
	xor_cursorY++;
	if (xor_lineSpacing) xor_cursorY++;
	xor_cursorX = 0;
}

void xor_PrintChar(char toPrint)
{
	if (toPrint == '\n')
	{
		xor_CRLF();
		return;
	}

	xor_Char(xor_cursorX * 8 + xor_clipX + 8, xor_cursorY * 8 + xor_clipY - 2, toPrint);
	xor_cursorX++;

	if (xor_cursorX >= xor_textCols) xor_CRLF();
}

void xor_Print(char const str[])
{
	for (unsigned char i = 0; str[i] != '\0'; i++) xor_PrintChar(str[i]);
}

void xor_PrintUInt8(unsigned char toPrint, unsigned char maxLength)
{
	unsigned char dividend = toPrint;
	bool leading = true;

	for (unsigned char divisor = intpow(10, maxLength - 1); divisor >= 1; divisor /= 10)
	{
		char nextChar = '0' + dividend / divisor;
		if (nextChar == '0' && leading && divisor > 1) nextChar = ' ';
		else leading = false;

		xor_PrintChar(nextChar);
		dividend %= divisor;
	}
}

void xor_PrintUInt8Tenths(unsigned char toPrint, unsigned char maxLength)
{
	xor_PrintUInt8(toPrint / 10, maxLength);
	xor_PrintChar('.');
	xor_PrintUInt8(toPrint % 10, 1);
}

void xor_PrintUInt24(unsigned int toPrint, unsigned char maxLength)
{
	unsigned int dividend = toPrint;
	bool leading = true;

	for (unsigned int divisor = intpow(10, maxLength - 1); divisor >= 1; divisor /= 10)
	{
		char nextChar = '0' + dividend / divisor;
		if (nextChar == '0' && leading && divisor > 1) nextChar = ' ';
		else leading = false;

		xor_PrintChar(nextChar);
		dividend %= divisor;
	}
}

void xor_PrintUInt24Adaptive(unsigned int toPrint)
{
	if (toPrint == 0)
	{
		xor_PrintChar('0');
	}
	else
	{
		unsigned char trialLength;
		for (trialLength = 8; intpow(10, trialLength - 1) > toPrint; trialLength--);
		xor_PrintUInt24(toPrint, trialLength);
	}
}

void xor_PrintUInt24Tenths(unsigned int toPrint)
{
	xor_PrintUInt24Adaptive(toPrint / 10);
	xor_PrintChar('.');
	xor_PrintUInt8(toPrint % 10, 1);
}
