#include <keypadc.h>

bool yequ, 
	 graph, 
	 up, 
	 down, 
	 left, 
	 right, 
	 enter, 
	 math, 
	 vars, 
	 clear;
unsigned char prevYequ, 
			  prevGraph, 
			  prevUp, 
			  prevDown, 
			  prevLeft, 
			  prevRight, 
			  prevEnter, 
			  prevMath, 
			  prevVars, 
			  prevClear;

char prevData[8];

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
	math = kb_IsDown(kb_KeyMath);
	vars = kb_IsDown(kb_KeyVars);
	clear = kb_IsDown(kb_KeyClear);
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
	prevMath = math ? prevMath + 1 : 0;
	prevVars = vars ? prevVars + 1 : 0;
	prevClear = clear ? prevClear + 1 : 0;
}

char getChar()
{
	// check for rising edges since last call
	char triggerData[8];
	for (unsigned char i = 1; i < 8; i++) triggerData[i] = kb_Data[i] & ~prevData[i];

	char output;

	if (triggerData[2] & kb_Math) output = 'A';
	else if (triggerData[3] & kb_Apps) output = 'B';
	else if (triggerData[4] & kb_Prgm) output = 'C';
	else if (triggerData[2] & kb_Recip) output = 'D';
	else if (triggerData[3] & kb_Sin) output = 'E';
	else if (triggerData[4] & kb_Cos) output = 'F';
	else if (triggerData[5] & kb_Tan) output = 'G';
	else if (triggerData[6] & kb_Power) output = 'H';
	else if (triggerData[2] & kb_Square) output = 'I';
	else if (triggerData[3] & kb_Comma) output = 'J';
	else if (triggerData[4] & kb_LParen) output = 'K';
	else if (triggerData[5] & kb_RParen) output = 'L';
	else if (triggerData[6] & kb_Div) output = 'M';
	else if (triggerData[2] & kb_Log) output = 'N';
	else if (triggerData[3] & kb_7) output = 'O';
	else if (triggerData[4] & kb_8) output = 'P';
	else if (triggerData[5] & kb_9) output = 'Q';
	else if (triggerData[6] & kb_Mul) output = 'R';
	else if (triggerData[2] & kb_Ln) output = 'S';
	else if (triggerData[3] & kb_4) output = 'T';
	else if (triggerData[4] & kb_5) output = 'U';
	else if (triggerData[5] & kb_6) output = 'V';
	else if (triggerData[6] & kb_Sub) output = 'W';
	else if (triggerData[2] & kb_Sto) output = 'X';
	else if (triggerData[3] & kb_1) output = 'Y';
	else if (triggerData[4] & kb_2) output = 'Z';
	else if (triggerData[6] & kb_Add) output = '\'';
	else if (triggerData[3] & kb_0) output = '-';
	else output = '\0';

	// update previous key states
	for (unsigned char i = 1; i < 8; i++) prevData[i] = kb_Data[i];

	return output;
}
