#include <keypadc.h>

unsigned char yequ, graph, up, down, left, right, 
			  enter, math, prgm, vars, clear, mode, 
			  del, graphVar, stat, apps, alpha, second;

char prevData[8];

void updateKeys()
{
	kb_Scan();

	yequ = kb_IsDown(kb_KeyYequ) ? yequ + 1 : 0;
	graph = kb_IsDown(kb_KeyGraph) ? graph + 1 : 0;
	up = kb_IsDown(kb_KeyUp) ? up + 1 : 0;
	down = kb_IsDown(kb_KeyDown) ? down + 1 : 0;
	left = kb_IsDown(kb_KeyLeft) ? left + 1 : 0;
	right = kb_IsDown(kb_KeyRight) ? right + 1 : 0;
	enter = kb_IsDown(kb_KeyEnter) ? enter + 1 : 0;
	math = kb_IsDown(kb_KeyMath) ? math + 1 : 0;
	prgm = kb_IsDown(kb_KeyPrgm) ? prgm + 1 : 0;
	vars = kb_IsDown(kb_KeyVars) ? vars + 1 : 0;
	clear = kb_IsDown(kb_KeyClear) ? clear + 1 : 0;
	mode = kb_IsDown(kb_KeyMode) ? mode + 1 : 0;
	del = kb_IsDown(kb_KeyDel) ? del + 1 : 0;
	graphVar = kb_IsDown(kb_KeyGraphVar) ? graphVar + 1 : 0;
	stat = kb_IsDown(kb_KeyStat) ? stat + 1 : 0;
	apps = kb_IsDown(kb_KeyApps) ? apps + 1 : 0;
	alpha = kb_IsDown(kb_KeyAlpha) ? alpha + 1 : 0;
	second = kb_IsDown(kb_Key2nd) ? second + 1 : 0;
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
