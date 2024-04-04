#ifndef input_include_file
#define input_include_file

extern bool yequ, graph, up, down, left, right, enter, math, prgm, vars, clear, mode, del, graphVar, stat;
extern unsigned char prevYequ, prevGraph, prevUp, prevDown, prevLeft, prevRight, prevEnter, prevMath, prevPrgm, prevVars, prevClear, prevMode, prevDel, prevGraphVar, prevStat;

void updateKeys();
void updatePrevKeys();

char getChar();

#endif
