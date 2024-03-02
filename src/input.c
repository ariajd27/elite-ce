#include <keypadc.h>

bool yequ, graph, up, down, left, right, enter;
unsigned char prevYequ, prevGraph, prevUp, prevDown, prevLeft, prevRight, prevEnter;

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
